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

/// @return The current TimerState/preset/remaining-time status, serialized as JSON, for /ws clients.
static String buildStatusJson() {
    JsonDocument doc;
    doc["state"] = timerStateName(timer.getState());
    doc["presetName"] = timer.getCurrentPresetName();
    doc["remainingMs"] = timer.getRemainingMillis();

    String json;
    serializeJson(doc, json);
    return json;
}

/// Handle /ws connection lifecycle events; a newly connected client is sent the current status
/// immediately rather than waiting for the next throttled webServerLoop() broadcast.
static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
                      size_t len) {
    (void) server;
    (void) arg;
    (void) data;
    (void) len;

    if (type == WS_EVT_CONNECT) {
        ESP_LOGI(TAG, "WebSocket client #%u connected", client->id());
        client->text(buildStatusJson());
    } else if (type == WS_EVT_DISCONNECT) {
        ESP_LOGI(TAG, "WebSocket client #%u disconnected", client->id());
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

void webServerLoop() {
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

    unsigned long now = millis();
    if (now - lastStatusBroadcast < STATUS_BROADCAST_INTERVAL) {
        return;
    }
    lastStatusBroadcast = now;

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
