/**
 * @file system_status.h
 * @brief One snapshot of the device's network, time, MQTT, hardware, firmware, storage and
 * statistics state, shared by the start screen's Status page and GET /system.
 */
#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <Arduino.h>
#include <ArduinoJson.h>

/// Everything the Status screens show. Collected by collectSystemStatus().
struct SystemStatus {
    // Network
    bool wifiConnected;  ///< Whether WiFi is connected with an IP address.
    String ssid;         ///< Configured network name.
    String ip;           ///< IP address, or "" while not connected.
    int rssi;            ///< Signal strength in dBm (0 while not connected).
    String mac;          ///< WiFi MAC address.

    // Time
    bool timeSynced;     ///< Whether SNTP has set the clock.
    String currentTime;  ///< Local time "YYYY-MM-DD HH:MM:SS", or "not synced yet".
    const char *sntpServer;
    const char *sntpTimezone;

    // MQTT
    bool mqttEnabled;  ///< false when no broker is configured (see config.h).
    bool mqttConnected;
    const char *mqttBrokerAddress;
    uint16_t mqttBrokerPort;

    // Device
    String chipModel;      ///< e.g. "ESP32-D0WD-V3".
    uint8_t chipRevision;  ///< Silicon revision.
    uint32_t cpuMhz;
    unsigned long uptimeSeconds;
    uint32_t freeHeapBytes;

    // Firmware
    const char *firmwareVersion;  ///< See version.h.
    const char *firmwareBuildTime;
    uint32_t firmwareBytes;      ///< Size of the running firmware image.
    uint32_t appPartitionBytes;  ///< Size of the partition it runs from (the firmware's flash budget).

    // Storage (LittleFS: the web pages in data/)
    size_t fsUsedBytes;   ///< 0 if LittleFS isn't mounted.
    size_t fsTotalBytes;  ///< 0 if LittleFS isn't mounted.

    // Lifetime statistics (see statistics.h)
    unsigned int totalCycles;
    unsigned long totalWorkMinutes;
    unsigned long totalBreakMinutes;
};

/// @return A fresh snapshot. Cheap enough to call once a second.
SystemStatus collectSystemStatus();

/**
 * @brief Serialize a snapshot into @p doc, as returned by GET /system.
 *
 * Keeps the keys settings.html and earlier clients already use (sntpServer, sntpTimezone,
 * timeSynced, currentTime, mqttEnabled, mqttBrokerAddress, mqttBrokerPort, mqttConnected).
 */
void systemStatusToJson(const SystemStatus &status, JsonDocument &doc);

/// @return @p bytes as a short human-readable size, e.g. "512 B", "37 KB", "1.4 MB".
String formatBytes(size_t bytes);

#endif  // SYSTEM_STATUS_H
