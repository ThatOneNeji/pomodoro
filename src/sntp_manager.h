/**
 * @file sntp_manager.h
 * @brief Thin wrapper around the ESP32 Arduino core's built-in SNTP client (configTzTime()), so
 * the device clock stays roughly correct while WiFi is connected. Purely informational for now —
 * nothing in the UI depends on the synced time yet.
 */
#ifndef SNTP_MANAGER_H
#define SNTP_MANAGER_H

#include <Arduino.h>

/**
 * @brief Start the SNTP client against ::SNTP_SERVER/::SNTP_TIMEZONE (see config.h).
 *
 * Non-blocking: configures the sync and returns immediately, same as WiFi.begin(). Call once
 * WiFi is confirmed connected (there's no point starting it earlier). There's no hardware RTC on
 * the ESP32, so the clock resets to the epoch on every reboot until this catches up again.
 */
void setupSntp();

/// @return true once the device clock has been set by SNTP (checked non-blockingly).
bool isTimeSynced();

/// @return The current local time as "YYYY-MM-DD HH:MM:SS", or "not synced yet" if isTimeSynced() is false.
String getCurrentTimeString();

#endif
