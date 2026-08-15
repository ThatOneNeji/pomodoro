/**
 * @file wifi_manager.h
 * @brief Resolves WiFi credentials: an NVS-saved override if the user has set one, otherwise the
 * compiled-in default from credentials.h.
 */
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

/// @return The SSID to connect to: an NVS override if setWifiCredentials() has been called, otherwise
/// WIFI_SSID_DEFAULT.
String getWifiSSID();

/// @return The WiFi password: an NVS override if setWifiCredentials() has been called, otherwise WIFI_PASSWORD_DEFAULT.
String getWifiPassword();

/**
 * @brief Persist a new SSID/password to NVS, taking precedence over the compiled-in default from then on.
 * @param ssid Network name.
 * @param password Network password.
 */
void setWifiCredentials(const String &ssid, const String &password);

#endif
