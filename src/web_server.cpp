/// @file web_server.cpp
#include "web_server.h"
// timer.h (and the Arduino/NeoPixelBus chain it pulls in) must be included before WiFi.h /
// ESPAsyncWebServer.h: the lwIP/socket-compat headers those pull in redefine macros (write,
// print, flush, ...) that collide with Stream/Print/HardwareSerial's real method names if
// NeoPixelBus's headers haven't been parsed yet.
#include "timer.h"
#include <WiFi.h>
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

/// Register routes and start listening. Safe to call more than once (e.g. on WiFi reconnect); only takes effect once.
static void startServer() {
    if (serverStarted) {
        return;
    }

    server.on("/status", HTTP_GET, handleStatus);
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
    WiFi.onEvent(onWiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    String ssid = getWifiSSID();
    ESP_LOGI(TAG, "Connecting to WiFi SSID \"%s\"...", ssid.c_str());
    WiFi.begin(ssid.c_str(), getWifiPassword().c_str());
}
