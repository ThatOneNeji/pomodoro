/// @file wifi_manager.cpp
#include "wifi_manager.h"
#include "preferences_manager.h"
#include "config.h"

String getWifiSSID() { return pref_getString("ssid", WIFI_SSID_DEFAULT); }

String getWifiPassword() { return pref_getString("password", WIFI_PASSWORD_DEFAULT); }

bool isWifiEnabled() { return pref_getCheckbox(WIFI_ENABLED_KEY, true); }

void setWifiEnabled(bool enabled) { pref_putCheckbox(WIFI_ENABLED_KEY, enabled); }

void setWifiCredentials(const String &ssid, const String &password) {
    pref_putString("ssid", ssid);
    pref_putString("password", password);
}
