/**
 * @file fonts.h
 * @brief Declares the bitmap fonts, which are defined exactly once, in fonts.cpp.
 *
 * The generated headers in src/fonts/ define their arrays as `const`, which in C++ gives every
 * .cpp file that includes them its own private copy. Including them from config.h (as before)
 * duplicated each font in every file that drew with it: ~34 KB of flash, growing with every new
 * screen. Only fonts.cpp may include the headers in src/fonts/.
 */
#ifndef FONTS_H
#define FONTS_H

#include <Adafruit_GFX.h>

extern const GFXfont FunnelDisplay_Bold24pt7b;     ///< See ::MAIN_FONT.
extern const GFXfont FunnelDisplay_Bold18pt7b;     ///< See ::SECONDARY_FONT.
extern const GFXfont FunnelDisplay_Regular14pt7b;  ///< See ::TEXT_FONT.
extern const GFXfont HelvetiPixel24pt7b;           ///< See ::SUB_FONT.
extern const GFXfont HelvetiPixel16pt7b;           ///< See ::SMALL_FONT.
extern const GFXfont FunnelDisplay_Bold48pt7b;     ///< See ::SEMI_LARGE_FONT.
extern const GFXfont FunnelDisplay_Bold32pt7b;     ///< See ::TITLE_FONT.
extern const GFXfont FunnelDisplay_Bold60pt7b;     ///< See ::LARGE_FONT.

#endif  // FONTS_H
