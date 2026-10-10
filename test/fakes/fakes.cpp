/**
 * @file fakes.cpp
 * @brief Host-test replacements for firmware parts that need the hardware: settings storage,
 * the LED, the button, fonts, and the Status and Settings screens.
 *
 * Built into `pio test -e native` only (see build_src_filter in platformio.ini).
 */
#include <Arduino.h>

#include "fakes.h"
#include "../../src/button.h"
#include "../../src/fonts.h"
#include "../../src/led.h"
#include "../../src/preferences_manager.h"
#include "../../src/settings_screen.h"
#include "../../src/status_screen.h"

// Settings: an in-memory map instead of flash.
Preferences preferences;
void initPreferences() {}
bool pref_getCheckbox(const char *key, bool defaultValue) {
    auto found = fake::prefs.find(key);
    return found == fake::prefs.end() ? defaultValue : found->second != 0;
}
void pref_putCheckbox(const char *key, bool value) {
    fake::prefs[key] = value;
    fake::prefWrites++;
}
unsigned int pref_getStatistic(const char *key, unsigned int defaultValue) {
    auto found = fake::prefs.find(key);
    return found == fake::prefs.end() ? defaultValue : found->second;
}
void pref_putStatistic(const char *key, unsigned int value) {
    fake::prefs[key] = value;
    fake::prefWrites++;
}
unsigned long pref_getStatistic(const char *key, unsigned long defaultValue) {
    auto found = fake::prefs.find(key);
    return found == fake::prefs.end() ? defaultValue : found->second;
}
void pref_putStatistic(const char *key, unsigned long value) {
    fake::prefs[key] = value;
    fake::prefWrites++;
}
String pref_getString(const char *, const String &defaultValue) { return defaultValue; }
void pref_putString(const char *, const String &) { fake::prefWrites++; }

// LED: nothing to light.
void setLedMode(LedMode) {}

// Button: pressed only when a test sets `Button::instance->pressed`.
Button *Button::instance = nullptr;
bool Button::instanceExists = false;
Button::Button(int pin) : pin(pin) {
    instance = this;
    instanceExists = true;
}
Button::~Button() {
    instance = nullptr;
    instanceExists = false;
}
bool Button::checkAndClearButtonPress() {
    const bool wasPressed = pressed;
    pressed = false;
    return wasPressed;
}

// Fonts: the fake display measures text without them.
const GFXfont FunnelDisplay_Bold24pt7b{};
const GFXfont FunnelDisplay_Bold18pt7b{};
const GFXfont FunnelDisplay_Regular14pt7b{};
const GFXfont HelvetiPixel24pt7b{};
const GFXfont HelvetiPixel16pt7b{};
const GFXfont FunnelDisplay_Bold48pt7b{};
const GFXfont FunnelDisplay_Bold32pt7b{};
const GFXfont FunnelDisplay_Bold60pt7b{};

// Status and Settings screens: return at once, as if dismissed straight away.
StatusScreen::StatusScreen(DISPLAY_CLASS &display) : display(display) {}
void StatusScreen::run(volatile const int *) {}
SettingsScreen::SettingsScreen(DISPLAY_CLASS &display) : display(display) {}
void SettingsScreen::run(volatile const int *) {}
