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
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
static const char *TAG = "WEBSERVER";

/// The global Timer instance, defined in main.cpp.
extern Timer timer;

static AsyncWebServer server(80);
static bool serverStarted = false;
/// Whether LittleFS mounted successfully; static file serving (data/) is skipped if not.
static bool littleFsMounted = false;

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

/// GET /status: current TimerState, active preset name (if any), and milliseconds remaining.
static void handleStatus(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["state"] = timerStateName(timer.getState());
    doc["presetName"] = timer.getCurrentPresetName();
    doc["remainingMs"] = timer.getRemainingMillis();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
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

    server.on("/status", HTTP_GET, handleStatus);
    server.on("/wifi", HTTP_GET, handleGetWifi);
    server.on("/wifi", HTTP_POST, handleSaveWifi);

    if (littleFsMounted) {
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }

    server.begin();
    serverStarted = true;
    ESP_LOGI(TAG, "Webserver started, status at http://%s/status", WiFi.localIP().toString().c_str());
}

static void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    (void) event;
    (void) info;
    ESP_LOGI(TAG, "WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());
    startServer();
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
