/// @file preferences_manager.cpp
#include "preferences_manager.h"
#include <nvs_flash.h>
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "PREFS";

Preferences preferences;

void initPreferences() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Partition layout or NVS format changed since it was last written; reformat and retry.
        ESP_LOGW(TAG, "NVS partition needs erase, reformatting");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

bool pref_getCheckbox(const char *key, bool defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_CHECKBOX, key);

    auto v = preferences.getBool(prefKey, defaultValue);

    preferences.end();

    return v;
}

void pref_putCheckbox(const char *key, bool value) {
    preferences.begin(PREFS_NAMESPACE, false);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_CHECKBOX, key);
    preferences.putBool(prefKey, value);
    preferences.end();
}

unsigned int pref_getStatistic(const char *key, unsigned int defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_STATISTICS, key);

    auto v = preferences.getUInt(prefKey, defaultValue);

    preferences.end();
    return v;
}

void pref_putStatistic(const char *key, unsigned int value) {
    preferences.begin(PREFS_NAMESPACE, false);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_STATISTICS, key);
    preferences.putUInt(prefKey, value);
    preferences.end();
}

unsigned long pref_getStatistic(const char *key, unsigned long defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_STATISTICS, key);

    auto v = preferences.getULong(prefKey, defaultValue);

    preferences.end();
    return v;
}

void pref_putStatistic(const char *key, unsigned long value) {
    preferences.begin(PREFS_NAMESPACE, false);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_STATISTICS, key);
    preferences.putULong(prefKey, value);
    preferences.end();
}

String pref_getString(const char *key, const String &defaultValue) {
    preferences.begin(PREFS_NAMESPACE, true);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_WIFI, key);

    auto v = preferences.getString(prefKey, defaultValue);

    preferences.end();
    return v;
}

void pref_putString(const char *key, const String &value) {
    preferences.begin(PREFS_NAMESPACE, false);
    char prefKey[32];
    snprintf(prefKey, sizeof(prefKey), "%s%s", PREF_WIFI, key);
    preferences.putString(prefKey, value);
    preferences.end();
}