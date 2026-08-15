/**
 * @file preferences_manager.h
 * @brief Thin, key-prefixed wrapper around the ESP32 NVS-backed Preferences library.
 *
 * Every call opens and closes the ::PREFS_NAMESPACE namespace around a single read/write; there
 * is no long-lived open handle. See initPreferences() for the one-time NVS bring-up this depends on.
 */
#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>

/// Preferences namespace all keys in this application are stored under.
#define PREFS_NAMESPACE "pomodoro"

/// Key prefix for Checkbox-backed boolean settings.
#define PREF_CHECKBOX "cb."
/// Key prefix for statistics counters (see statistics.h).
#define PREF_STATISTICS "stats."
/// Key prefix for anniversary-mode state.
#define PREF_ANNIVERSARY "anniv."
/// Key prefix for WiFi settings (see wifi_manager.h).
#define PREF_WIFI "wifi."

/// Shared Preferences handle used by every pref_* accessor.
extern Preferences preferences;

/**
 * @brief One-time NVS bring-up. Must be called before any pref_* accessor is used.
 *
 * Calls `nvs_flash_init()` and, if the stored NVS data doesn't match the current partition
 * layout/format, erases and reinitializes it so subsequent Preferences calls don't fail with
 * `NOT_INITIALIZED`.
 */
void initPreferences();

/**
 * @brief Read a persisted boolean setting.
 * @param key Key, relative to the ::PREF_CHECKBOX prefix.
 * @param defaultValue Value returned if @p key has never been stored.
 * @return The stored value, or @p defaultValue.
 */
bool pref_getCheckbox(const char *key, bool defaultValue);

/**
 * @brief Persist a boolean setting.
 * @param key Key, relative to the ::PREF_CHECKBOX prefix.
 * @param value Value to store.
 */
void pref_putCheckbox(const char *key, bool value);

/**
 * @brief Read a persisted `unsigned int` statistic.
 * @param key Key, relative to the ::PREF_STATISTICS prefix.
 * @param defaultValue Value returned if @p key has never been stored.
 * @return The stored value, or @p defaultValue.
 */
unsigned int pref_getStatistic(const char *key, unsigned int defaultValue);

/**
 * @brief Persist an `unsigned int` statistic.
 * @param key Key, relative to the ::PREF_STATISTICS prefix.
 * @param value Value to store.
 */
void pref_putStatistic(const char *key, unsigned int value);

/**
 * @brief Read a persisted `unsigned long` statistic.
 * @param key Key, relative to the ::PREF_STATISTICS prefix.
 * @param defaultValue Value returned if @p key has never been stored.
 * @return The stored value, or @p defaultValue.
 */
unsigned long pref_getStatistic(const char *key, unsigned long defaultValue);

/**
 * @brief Persist an `unsigned long` statistic.
 * @param key Key, relative to the ::PREF_STATISTICS prefix.
 * @param value Value to store.
 */
void pref_putStatistic(const char *key, unsigned long value);

/**
 * @brief Read a persisted string setting.
 * @param key Key, relative to the ::PREF_WIFI prefix.
 * @param defaultValue Value returned if @p key has never been stored.
 * @return The stored value, or @p defaultValue.
 */
String pref_getString(const char *key, const String &defaultValue);

/**
 * @brief Persist a string setting.
 * @param key Key, relative to the ::PREF_WIFI prefix.
 * @param value Value to store.
 */
void pref_putString(const char *key, const String &value);

#endif