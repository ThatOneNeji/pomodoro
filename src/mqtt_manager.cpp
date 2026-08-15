/// @file mqtt_manager.cpp
#include "mqtt_manager.h"
#include "config.h"
#include "mqtt_credentials.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
static const char *TAG = "MQTT";

/// Client ID presented to the broker; fixed since this device doesn't run more than one instance.
static const char *MQTT_CLIENT_ID = "esp32-pomodoro";
/// Minimum time between reconnect attempts, so a down/unreachable broker doesn't stall loop() repeatedly.
static const unsigned long RECONNECT_INTERVAL = 5000;

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
/// Whether MQTT is configured at all (MQTT_BROKER_ADDRESS non-empty); set once by setupMqtt().
static bool mqttEnabled = false;
/// millis() timestamp of the last connect attempt, throttling mqttLoop()'s reconnect attempts.
static unsigned long lastConnectAttempt = 0;

void setupMqtt() {
    mqttEnabled = strlen(MQTT_BROKER_ADDRESS) > 0;
    if (!mqttEnabled) {
        ESP_LOGI(TAG, "MQTT_BROKER_ADDRESS is empty; MQTT client disabled");
        return;
    }

    mqttClient.setServer(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
    ESP_LOGI(TAG, "MQTT client configured for %s:%u", MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
}

/// Attempt a single (blocking, PubSubClient-internal-timeout-bounded) connect to the broker.
static void tryConnect() {
    ESP_LOGI(TAG, "Connecting to MQTT broker %s:%u...", MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);

    bool connected = strlen(MQTT_USERNAME) > 0 ? mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)
                                               : mqttClient.connect(MQTT_CLIENT_ID);

    if (connected) {
        ESP_LOGI(TAG, "MQTT connected");
    } else {
        ESP_LOGW(TAG, "MQTT connect failed, rc=%d", mqttClient.state());
    }
}

void mqttLoop() {
    if (!mqttEnabled || WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (mqttClient.connected()) {
        mqttClient.loop();
        return;
    }

    unsigned long now = millis();
    if (now - lastConnectAttempt < RECONNECT_INTERVAL) {
        return;
    }
    lastConnectAttempt = now;

    tryConnect();
}

bool isMqttConnected() { return mqttEnabled && mqttClient.connected(); }
