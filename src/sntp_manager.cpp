/// @file sntp_manager.cpp
#include "sntp_manager.h"
#include "config.h"
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
static const char *TAG = "SNTP";

void setupSntp() {
    ESP_LOGI(TAG, "Starting SNTP sync against %s (tz \"%s\")", SNTP_SERVER, SNTP_TIMEZONE);
    configTzTime(SNTP_TIMEZONE, SNTP_SERVER);
}

bool isTimeSynced() {
    struct tm timeinfo;
    // ms=0: check whether the time has already been set, without blocking loop() to wait for it.
    return getLocalTime(&timeinfo, 0);
}

String getCurrentTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) {
        return "not synced yet";
    }

    char buffer[24];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}
