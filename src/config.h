/**
 * @file config.h
 * @brief Board wiring (e-paper SPI pins) and user-facing build-time options.
 */
#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>

#include "fonts/FunnelDisplay_Bold24pt7b.h"
#include "fonts/FunnelDisplay_Bold18pt7b.h"
#include "fonts/FunnelDisplay_Regular14pt7b.h"
#include "fonts/HelvetiPixel24pt7b.h"
#include "fonts/HelvetiPixel16pt7b.h"
#include "fonts/FunnelDisplay_Bold48pt7b.h"
#include "fonts/FunnelDisplay_Bold32pt7b.h"
#include "fonts/FunnelDisplay_Bold60pt7b.h"

/// Whether the one-off Anniversary presentation (see anniversary.h) is compiled in. Currently unimplemented.
#define ANNIVERSARY_MODE false

/// @name Named font aliases
/// Semantic names for the GFXfont bitmap fonts, used throughout the UI drawing code.
/// @{
#define TITLE_FONT FunnelDisplay_Bold32pt7b       ///< Screen/section titles.
#define MAIN_FONT FunnelDisplay_Bold24pt7b        ///< Primary content text (e.g. preset names).
#define TEXT_FONT FunnelDisplay_Regular14pt7b     ///< General body text.
#define SECONDARY_FONT FunnelDisplay_Bold18pt7b   ///< Secondary/supporting text.
#define SUB_FONT HelvetiPixel24pt7b               ///< Menu bar and subtitle text.
#define SMALL_FONT HelvetiPixel16pt7b             ///< Small/compact text (e.g. speech-bubble messages).
#define SEMI_LARGE_FONT FunnelDisplay_Bold48pt7b  ///< Large emphasis text.
#define LARGE_FONT FunnelDisplay_Bold60pt7b       ///< Largest text (e.g. the countdown display).
/// @}

/// Slow display redraw interval, in milliseconds.
#define REDRAW_INTERVAL_SLOW 1000

/// Concrete GxEPD2 display type in use (480px-wide 4.26" GDEQ0426T82 panel), used wherever display references are
/// typed.
#define DISPLAY_CLASS GxEPD2_BW<GxEPD2_426_GDEQ0426T82, 480>

#define MINUTE 60 * 1000  ///< One minute, in milliseconds; used to size preset durations.

#define ENCODER_CLK 32  ///< Encoder clock (A) pin.
#define ENCODER_DT 21   ///< Encoder data (B) pin.

#define ENCODER_STABILITY_DELAY 50          ///< ms to wait after a button press before trusting encoder changes.
#define ENCODER_LOCK_TIME_AFTER_BUTTON 200  ///< Unused; see ENCODER_STABILITY_DELAY, which is what's actually applied.

#define ENCODER_SW 14  ///< Optional encoder push-button pin, passed to Button's constructor in main.cpp.

/// @name E-paper display pin wiring
/// @{
#define EPD_SCK_PIN 18   ///< SPI clock pin.
#define EPD_MOSI_PIN 23  ///< SPI MOSI (data) pin.
#define EPD_CS_PIN 5     ///< SPI chip-select pin.
#define EPD_RST_PIN 16   ///< Display reset pin.
#define EPD_DC_PIN 17    ///< Data/command select pin.
#define EPD_BUSY_PIN 4   ///< Display busy-status input pin.
#define EPD_PWR_PIN 22   ///< Display power-enable output pin.
/// @}

/**
 * @brief Change this to 'true' to use the old style of displaying the remaining time
 *
 */
const static bool timerStyleOld = false;

// #define LANG_AF // Uncomment for Afrikaans text messages
// #define LANG_DE // Uncomment for German text messages
