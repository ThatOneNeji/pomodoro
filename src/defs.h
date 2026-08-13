/**
 * @file defs.h
 * @brief Project-wide constants: the concrete display type and named font/interval aliases.
 */
#ifndef DEFS_H
#define DEFS_H

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

#endif
