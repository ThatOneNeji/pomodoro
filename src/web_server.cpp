/// @file web_server.cpp
#include "web_server.h"
// timer.h (and the Arduino/NeoPixelBus chain it pulls in) must be included before WiFi.h /
// ESPAsyncWebServer.h: the lwIP/socket-compat headers those pull in redefine macros (write,
// print, flush, ...) that collide with Stream/Print/HardwareSerial's real method names if
// NeoPixelBus's headers haven't been parsed yet.
#include "timer.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi_manager.h"
#include "sntp_manager.h"
#include "mqtt_manager.h"
#include "esp_log.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/// Log tag for this file, used by ESP_LOGx() calls.
static const char *TAG = "WEBSERVER";

/// The global Timer instance, defined in main.cpp.
extern Timer timer;

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static bool serverStarted = false;
/// Whether LittleFS mounted successfully; static file serving (data/) is skipped if not.
static bool littleFsMounted = false;
/// millis() timestamp status was last broadcast to /ws clients, throttling webServerLoop().
static unsigned long lastStatusBroadcast = 0;
/// How often to broadcast status to /ws clients, in ms.
static const unsigned long STATUS_BROADCAST_INTERVAL = 1000;
/// State and preset index as of the last status broadcast; a change triggers an immediate one.
static TimerState lastBroadcastState = TimerState::SelectingPreset;
static unsigned int lastBroadcastPresetIndex = 0;

/// A /ws control command, queued by the network task and applied by the main loop().
struct PendingCommand {
    TimerCommand command;
    int arg;
};
/// Hands commands from the AsyncTCP task (onWsEvent) to loop() (webServerNextCommand()). The timer
/// and display aren't thread-safe, so the network task must never call into Timer directly.
static QueueHandle_t commandQueue = nullptr;
static const UBaseType_t COMMAND_QUEUE_LENGTH = 8;
/// Set once loop() is running (first webServerLoop() call). Commands sent earlier, e.g. while the
/// blocking splash screen is up, are rejected rather than replayed later out of context.
static std::atomic<bool> commandsEnabled{false};
/// Set when a client connects; webServerLoop() then sends the preset list and current status.
/// Sending from loop() rather than the connect callback avoids reading Timer from the network
/// task, including during setup() while presets are still being added.
static std::atomic<bool> welcomePending{false};

/// @return A human-readable name for a TimerState, for JSON responses.
static const char *timerStateName(TimerState state) {
    switch (state) {
        case TimerState::SelectingPreset:
            return "SelectingPreset";
        case TimerState::Running:
            return "Running";
        case TimerState::WaitingConfirmStartOfBreak:
            return "WaitingConfirmStartOfBreak";
        case TimerState::RunningBreak:
            return "RunningBreak";
        case TimerState::WaitingConfirmEndOfBreak:
            return "WaitingConfirmEndOfBreak";
        case TimerState::UserInitiatedPause:
            return "UserInitiatedPause";
        case TimerState::UserInitiatedBreakPause:
            return "UserInitiatedBreakPause";
        case TimerState::Stopped:
            return "Stopped";
        default:
            return "Unknown";
    }
}

/// @return The current timer status, serialized as a `"type": "status"` JSON message for /ws clients.
static String buildStatusJson() {
    JsonDocument doc;
    doc["type"] = "status";
    doc["state"] = timerStateName(timer.getState());
    doc["presetIndex"] = timer.getPresetIndex();
    doc["presetName"] = timer.getCurrentPresetName();
    doc["remainingMs"] = timer.getRemainingMillis();
    doc["durationMs"] = timer.getIntervalMillis();
    doc["isLongBreak"] = timer.isOnLongBreak();
    doc["cycles"] = timer.getCycles();

    // endsAt lets clients keep counting down on their own between messages (or through a dropped
    // connection); deviceTime lets them correct for their own clock differing from the device's.
    // Both are null until SNTP has synced, and endsAt is also null unless an interval is actively
    // counting down (paused or idle), in which case clients should show remainingMs as-is.
    int64_t nowMillis;
    const bool synced = getEpochMillis(nowMillis);
    const TimerState state = timer.getState();
    const bool countingDown = state == TimerState::Running || state == TimerState::RunningBreak;
    if (synced) {
        doc["deviceTime"] = nowMillis;
    } else {
        doc["deviceTime"] = nullptr;
    }
    if (synced && countingDown) {
        doc["endsAt"] = nowMillis + (int64_t) timer.getRemainingMillis();
    } else {
        doc["endsAt"] = nullptr;
    }

    String json;
    serializeJson(doc, json);
    return json;
}

/// @return The registered presets, serialized as a `"type": "presets"` JSON message for /ws clients.
static String buildPresetsJson() {
    JsonDocument doc;
    doc["type"] = "presets";
    JsonArray presets = doc["presets"].to<JsonArray>();
    for (unsigned int i = 0; i < timer.getPresetCount(); i++) {
        Preset *preset = timer.getPreset(i);
        JsonObject entry = presets.add<JsonObject>();
        entry["name"] = preset->getName();
        entry["durationMs"] = preset->getDuration();
        entry["pauseMs"] = preset->getPauseDuration();
        entry["longPauseMs"] = preset->getLongPauseDuration();
        entry["longPauseAfter"] = preset->getLongPauseAfter();
    }

    String json;
    serializeJson(doc, json);
    return json;
}

/// Send a `"type": "error"` message to one client.
static void sendError(AsyncWebSocketClient *client, const char *message) {
    JsonDocument doc;
    doc["type"] = "error";
    doc["error"] = message;

    String json;
    serializeJson(doc, json);
    client->text(json);
}

/// Maps the `cmd` field of a /ws message to a TimerCommand. @return false if unknown.
static bool parseCommandName(const char *name, TimerCommand &command) {
    static const struct {
        const char *name;
        TimerCommand command;
    } COMMANDS[] = {
        {"select", TimerCommand::Select}, {"start", TimerCommand::Start},     {"pause", TimerCommand::Pause},
        {"resume", TimerCommand::Resume}, {"restart", TimerCommand::Restart}, {"stop", TimerCommand::Stop},
        {"break", TimerCommand::Break},
    };

    for (const auto &entry : COMMANDS) {
        if (strcmp(name, entry.name) == 0) {
            command = entry.command;
            return true;
        }
    }
    return false;
}

/**
 * @brief Parse one /ws text message and queue it for loop(). Runs on the AsyncTCP task.
 *
 * Message: `{"cmd": "select"|"start"|"pause"|"resume"|"restart"|"stop"|"break", "preset": <index>}`
 * (`preset` only for `select`). Malformed or unknown commands get a `"type": "error"` reply.
 * Whether a command is valid in the current state is only known once loop() applies it; the
 * resulting status change is broadcast to every client either way.
 */
static void handleWsMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    auto *info = static_cast<AwsFrameInfo *>(arg);
    // Commands are tiny; only accept complete, single-frame text messages.
    if (!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) {
        sendError(client, "unsupported frame");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, data, len)) {
        sendError(client, "invalid JSON");
        return;
    }

    PendingCommand pending;
    if (!parseCommandName(doc["cmd"] | "", pending.command)) {
        sendError(client, "unknown cmd");
        return;
    }
    pending.arg = doc["preset"] | -1;
    if (pending.command == TimerCommand::Select && pending.arg < 0) {
        sendError(client, "select needs a preset index");
        return;
    }

    if (!commandsEnabled) {
        sendError(client, "not ready");
        return;
    }
    if (xQueueSend(commandQueue, &pending, 0) != pdTRUE) {
        sendError(client, "busy");
        return;
    }
    ESP_LOGI(TAG, "WebSocket client #%u queued cmd \"%s\"", client->id(), doc["cmd"].as<const char *>());
}

/// Handle /ws events. Never reads Timer here (see ::welcomePending); commands are queued for loop().
static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
                      size_t len) {
    (void) server;

    if (type == WS_EVT_CONNECT) {
        ESP_LOGI(TAG, "WebSocket client #%u connected", client->id());
        welcomePending = true;
    } else if (type == WS_EVT_DISCONNECT) {
        ESP_LOGI(TAG, "WebSocket client #%u disconnected", client->id());
    } else if (type == WS_EVT_DATA) {
        handleWsMessage(client, arg, data, len);
    }
}

/// GET /wifi: the currently configured SSID. Never returns the password.
static void handleGetWifi(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["ssid"] = getWifiSSID();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

/**
 * @brief GET /system: read-only SNTP/MQTT configuration and current status, for settings.html's
 * info panels. Nothing here is editable yet (see roadmap.MD) — this just surfaces the config.h
 * defaults and current sync/connection state.
 */
static void handleGetSystem(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["sntpServer"] = SNTP_SERVER;
    doc["sntpTimezone"] = SNTP_TIMEZONE;
    doc["timeSynced"] = isTimeSynced();
    doc["currentTime"] = getCurrentTimeString();

    bool mqttEnabled = strlen(MQTT_BROKER_ADDRESS) > 0;
    doc["mqttEnabled"] = mqttEnabled;
    doc["mqttBrokerAddress"] = MQTT_BROKER_ADDRESS;
    doc["mqttBrokerPort"] = MQTT_BROKER_PORT;
    doc["mqttConnected"] = isMqttConnected();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

/**
 * @brief POST /wifi: save new WiFi credentials, then restart so they take effect.
 *
 * Body: `{"ssid": "...", "password": "..."}`. `password` may be omitted (or empty) to keep the
 * currently saved password unchanged, so the settings page never has to know/display it. `ssid`
 * is required and rejected with 400 if blank, to avoid saving a configuration that can never
 * reconnect. On success, responds first and only then restarts (mirrors the same
 * save-then-ESP.restart() pattern SplashScreen::loopSettings() already uses on-device), giving
 * the response time to actually reach the browser before the connection drops.
 */
static void handleSaveWifi(AsyncWebServerRequest *request, const JsonVariant &json) {
    String ssid = json["ssid"] | "";
    if (ssid.length() == 0) {
        request->send(400, "application/json", "{\"error\":\"ssid must not be empty\"}");
        return;
    }

    String password = json["password"] | "";
    if (password.length() == 0) {
        password = getWifiPassword();
    }

    setWifiCredentials(ssid, password);
    ESP_LOGI(TAG, "Saved new WiFi credentials for SSID \"%s\"; restarting", ssid.c_str());

    request->send(200, "application/json", "{\"ok\":true}");
    delay(1000);
    ESP.restart();
}

/// Register routes and start listening. Safe to call more than once (e.g. on WiFi reconnect); only takes effect once.
static void startServer() {
    if (serverStarted) {
        return;
    }

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    server.on("/wifi", HTTP_GET, handleGetWifi);
    server.on("/wifi", HTTP_POST, handleSaveWifi);
    server.on("/system", HTTP_GET, handleGetSystem);

    if (littleFsMounted) {
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }

    server.begin();
    serverStarted = true;
    ESP_LOGI(TAG, "Webserver started, status feed at ws://%s/ws", WiFi.localIP().toString().c_str());
}

bool webServerNextCommand(TimerCommand &command, int &arg) {
    PendingCommand pending;
    if (commandQueue == nullptr || xQueueReceive(commandQueue, &pending, 0) != pdTRUE) {
        return false;
    }
    command = pending.command;
    arg = pending.arg;
    return true;
}

void webServerLoop() {
    commandsEnabled = true;

    if (!serverStarted) {
        return;
    }

    // ESPAsyncWebServer recommends periodically pruning clients that disconnected without a
    // clean close handshake, since it can't detect that on its own; doing it here means it
    // happens once a loop() tick without needing a separate timer.
    ws.cleanupClients();

    if (ws.count() == 0) {
        return;
    }

    // New clients get the preset list once, then the status below. Sent to all clients for
    // simplicity; existing clients just re-render the same list.
    const bool welcome = welcomePending.exchange(false);
    if (welcome) {
        ws.textAll(buildPresetsJson());
    }

    // Broadcast immediately on a state/preset change (from the encoder or a /ws command), and
    // otherwise once a second for the countdown.
    const TimerState state = timer.getState();
    const unsigned int presetIndex = timer.getPresetIndex();
    const bool changed = state != lastBroadcastState || presetIndex != lastBroadcastPresetIndex;

    unsigned long now = millis();
    if (!welcome && !changed && now - lastStatusBroadcast < STATUS_BROADCAST_INTERVAL) {
        return;
    }
    lastStatusBroadcast = now;
    lastBroadcastState = state;
    lastBroadcastPresetIndex = presetIndex;

    ws.textAll(buildStatusJson());
}

static void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    (void) event;
    (void) info;
    ESP_LOGI(TAG, "WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());
    startServer();
    setupSntp();
}

static void onWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    (void) event;
    (void) info;
    ESP_LOGW(TAG, "WiFi disconnected");
}

void setupWebServer() {
    commandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(PendingCommand));

    // Mount is local and doesn't depend on WiFi; do it up front so a missing/corrupt filesystem
    // (e.g. `pio run -t uploadfs` was never run) is logged immediately rather than only once a
    // request for a static file 404s later.
    littleFsMounted = LittleFS.begin(true);
    if (littleFsMounted) {
        ESP_LOGI(TAG, "LittleFS mounted");
    } else {
        ESP_LOGW(TAG, "LittleFS mount failed; static files won't be served. Did you run 'pio run -t uploadfs'?");
    }

    WiFi.onEvent(onWiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    String ssid = getWifiSSID();
    ESP_LOGI(TAG, "Connecting to WiFi SSID \"%s\"...", ssid.c_str());
    WiFi.begin(ssid.c_str(), getWifiPassword().c_str());
}
