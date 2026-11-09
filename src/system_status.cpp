/// @file system_status.cpp
#include "system_status.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <esp_ota_ops.h>
#include "config.h"
#include "wifi_manager.h"
#include "sntp_manager.h"
#include "mqtt_manager.h"
#include "statistics.h"
#include "version.h"

SystemStatus collectSystemStatus() {
    SystemStatus s;

    s.wifiEnabled = isWifiEnabled();
    s.wifiConnected = WiFi.status() == WL_CONNECTED;
    s.ssid = getWifiSSID();
    s.ip = s.wifiConnected ? WiFi.localIP().toString() : String();
    s.rssi = s.wifiConnected ? WiFi.RSSI() : 0;
    // Read from the chip, not the WiFi driver, which doesn't know it while WiFi is switched off.
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macText[18];
    snprintf(macText, sizeof(macText), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    s.mac = macText;
    s.hostname = strlen(MDNS_HOSTNAME) > 0 ? String(MDNS_HOSTNAME) + ".local" : String();

    s.timeSynced = isTimeSynced();
    s.currentTime = getCurrentTimeString();
    s.sntpServer = SNTP_SERVER;
    s.sntpTimezone = SNTP_TIMEZONE;

    s.mqttEnabled = strlen(MQTT_BROKER_ADDRESS) > 0;
    s.mqttConnected = isMqttConnected();
    s.mqttBrokerAddress = MQTT_BROKER_ADDRESS;
    s.mqttBrokerPort = MQTT_BROKER_PORT;

    s.chipModel = ESP.getChipModel();
    s.chipRevision = ESP.getChipRevision();
    s.cpuMhz = ESP.getCpuFreqMHz();
    s.uptimeSeconds = millis() / 1000;
    s.freeHeapBytes = ESP.getFreeHeap();
    s.totalHeapBytes = ESP.getHeapSize();
    s.minFreeHeapBytes = ESP.getMinFreeHeap();

    s.firmwareVersion = getFirmwareVersion();
    s.firmwareBuildTime = getFirmwareBuildTime();
    s.firmwareBytes = ESP.getSketchSize();
    // ESP.getFreeSketchSpace() reports the *next OTA* partition, which doesn't exist with this
    // project's no_ota.csv partition table; the running partition is the real budget.
    const esp_partition_t *running = esp_ota_get_running_partition();
    s.appPartitionBytes = running ? running->size : 0;

    // totalBytes() is 0 when LittleFS isn't mounted (e.g. `pio run -t uploadfs` never ran).
    s.fsTotalBytes = LittleFS.totalBytes();
    s.fsUsedBytes = s.fsTotalBytes > 0 ? LittleFS.usedBytes() : 0;

    getStatistics(&s.totalCycles, &s.totalWorkMinutes, &s.totalBreakMinutes);
    return s;
}

void systemStatusToJson(const SystemStatus &s, JsonDocument &doc) {
    doc["wifiEnabled"] = s.wifiEnabled;
    doc["wifiConnected"] = s.wifiConnected;
    doc["ssid"] = s.ssid;
    doc["ip"] = s.ip;
    doc["rssi"] = s.rssi;
    doc["mac"] = s.mac;
    doc["hostname"] = s.hostname;

    doc["timeSynced"] = s.timeSynced;
    doc["currentTime"] = s.currentTime;
    doc["sntpServer"] = s.sntpServer;
    doc["sntpTimezone"] = s.sntpTimezone;

    doc["mqttEnabled"] = s.mqttEnabled;
    doc["mqttConnected"] = s.mqttConnected;
    doc["mqttBrokerAddress"] = s.mqttBrokerAddress;
    doc["mqttBrokerPort"] = s.mqttBrokerPort;

    doc["chipModel"] = s.chipModel;
    doc["chipRevision"] = s.chipRevision;
    doc["cpuMhz"] = s.cpuMhz;
    doc["uptimeSeconds"] = s.uptimeSeconds;
    doc["freeHeapBytes"] = s.freeHeapBytes;
    doc["totalHeapBytes"] = s.totalHeapBytes;
    doc["minFreeHeapBytes"] = s.minFreeHeapBytes;

    doc["firmwareVersion"] = s.firmwareVersion;
    doc["firmwareBuildTime"] = s.firmwareBuildTime;
    doc["firmwareBytes"] = s.firmwareBytes;
    doc["appPartitionBytes"] = s.appPartitionBytes;

    doc["fsUsedBytes"] = s.fsUsedBytes;
    doc["fsTotalBytes"] = s.fsTotalBytes;

    doc["totalCycles"] = s.totalCycles;
    doc["totalWorkMinutes"] = s.totalWorkMinutes;
    doc["totalBreakMinutes"] = s.totalBreakMinutes;
}

String formatBytes(size_t bytes) {
    char buffer[16];
    if (bytes < 1024) {
        snprintf(buffer, sizeof(buffer), "%u B", (unsigned) bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%u KB", (unsigned) ((bytes + 512) / 1024));
    } else {
        const unsigned tenths = (unsigned) ((bytes * 10ULL + 512 * 1024) / (1024 * 1024));
        snprintf(buffer, sizeof(buffer), "%u.%u MB", tenths / 10, tenths % 10);
    }
    return String(buffer);
}
