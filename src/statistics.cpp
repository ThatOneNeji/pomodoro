/// @file statistics.cpp
#include "statistics.h"
#include "preferences_manager.h"
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "STATS";

extern Preferences preferences;

void incrementTotalCycles() {
    ESP_LOGI(TAG, "incrementTotalCycles (+1)");
    unsigned int totalCycles = pref_getStatistic("tcs", (unsigned int) 0);
    totalCycles++;
    pref_putStatistic("tcs", totalCycles);
}

void incrementTotalTime(unsigned long ms) {
    ESP_LOGI(TAG, "incrementTotalTime (%lu [+%lu])", ms, ms / 1000 / 60);
    unsigned long totalTime = pref_getStatistic("tt", (unsigned long) 0);
    totalTime += ms / 1000 / 60;
    pref_putStatistic("tt", totalTime);
}

void incrementTotalBreakTime(unsigned long ms) {
    ESP_LOGI(TAG, "incrementTotalBreakTime (%lu [+%lu])", ms, ms / 1000 / 60);
    unsigned long totalBreakTime = pref_getStatistic("tbt", (unsigned long) 0);
    totalBreakTime += ms / 1000 / 60;
    pref_putStatistic("tbt", totalBreakTime);
}

void getStatistics(unsigned int *totalCycles, unsigned long *totalTime, unsigned long *totalBreakTime) {
    *totalCycles = pref_getStatistic("tcs", (unsigned int) 0);
    *totalTime = pref_getStatistic("tt", (unsigned long) 0);
    *totalBreakTime = pref_getStatistic("tbt", (unsigned long) 0);

    ESP_LOGD(TAG, "getStatistics: totalCycles=%u, totalTime=%lu, totalBreakTime=%lu", *totalCycles, *totalTime,
             *totalBreakTime);
}

void resetStatistics() {
    ESP_LOGI(TAG, "resetStatistics");
    pref_putStatistic("tcs", (unsigned int) 0);
    pref_putStatistic("tt", (unsigned long) 0);
    pref_putStatistic("tbt", (unsigned long) 0);
}