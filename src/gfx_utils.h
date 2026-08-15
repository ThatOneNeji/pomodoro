/**
 * @file gfx_utils.h
 * @brief Drawing helpers layered on top of GxEPD2/Adafruit_GFX: text with bounds, fill patterns, progress bars.
 */
#ifndef GFXUTILS_H
#define GFXUTILS_H

#include <Arduino.h>
#include "config.h"
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include <vector>

/// Visual style for drawProgressBar().
enum class ProgressBarStyle {
    Bordered,   ///< Outlined bar with the fill inset inside a border.
    Borderless  ///< Solid fill with no separate border.
};

/// Selects an 8x8 fill pattern for drawPattern()/drawPatternInRoundedArea(), indexing into ::patterns.
enum class Pattern { Solid, Stripes, Dots, Checkerboard, DiagonalStripes, CrossHatch, SparseDots, VerySparseDots };

// define 8x8 patterns
/// Solid fill. Indexed by Pattern::Solid.
const uint8_t pattern_solid[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
/// Wide horizontal bands. Indexed by Pattern::Stripes.
const uint8_t pattern_stripes[8] = {0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F};
/// Soft dot grid. Indexed by Pattern::Dots.
const uint8_t pattern_dots[8] = {0x88, 0x44, 0x22, 0x11, 0x11, 0x22, 0x44, 0x88};
/// Widely-spaced dots. Indexed by Pattern::SparseDots.
const uint8_t pattern_sparse_dots[8] = {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00};
/// Very widely-spaced dots. Indexed by Pattern::VerySparseDots.
const uint8_t pattern_very_sparse_dots[8] = {0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00};
/// Alternating-bit checkerboard. Indexed by Pattern::Checkerboard.
const uint8_t pattern_checkerboard[8] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
/// Repeated diagonal bands. Indexed by Pattern::DiagonalStripes.
const uint8_t pattern_diagonal_stripes[8] = {0xC0, 0x30, 0x0C, 0x03, 0xC0, 0x30, 0x0C, 0x03};
/// Grid-like pattern with full horizontal bars top, middle, and bottom. Indexed by Pattern::CrossHatch.
const uint8_t pattern_crosshatch[8] = {0xFF, 0x92, 0x92, 0x92, 0xFF, 0x92, 0x92, 0xFF};

/// Pattern bitmaps indexed by ::Pattern.
const std::vector<const uint8_t *> patterns = {pattern_solid,        pattern_stripes,          pattern_dots,
                                               pattern_checkerboard, pattern_diagonal_stripes, pattern_crosshatch,
                                               pattern_sparse_dots,  pattern_very_sparse_dots};

/**
 * @brief Draw a small crosshair at a point, for visually checking layout math. Compiled out unless DEBUG is defined.
 * @param display Target display.
 * @param x Crosshair center X.
 * @param y Crosshair center Y.
 * @param length Half-length of each arm, in pixels.
 * @param color Line color.
 */
void drawDebugCrosshair(DISPLAY_CLASS &display, int16_t x, int16_t y, int16_t length = 8, uint16_t color = GxEPD_BLACK);

/**
 * @brief Fill a rectangular region with a tiled 8x8 pattern.
 * @param display Target display.
 * @param pattern Which pattern to tile.
 * @param x Left edge of the region.
 * @param y Top edge of the region.
 * @param w Region width.
 * @param h Region height.
 */
void drawPattern(DISPLAY_CLASS &display, Pattern pattern, int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Fill a rounded-rectangle region with a tiled pattern, clipped to the rounded corners.
 * @param display Target display.
 * @param startX Left edge of the region.
 * @param startY Top edge of the region.
 * @param areaWidth Region width.
 * @param areaHeight Region height.
 * @param radius Corner radius.
 * @param patternNo Which pattern to tile.
 */
void drawPatternInRoundedArea(DISPLAY_CLASS &display, int16_t startX, int16_t startY, int16_t areaWidth,
                              int16_t areaHeight, int16_t radius, Pattern patternNo);

/**
 * @brief Draw a rounded progress bar.
 * @param display Target display.
 * @param style Bordered (patterned fill inset within an outline) or Borderless (solid fill).
 * @param x Left edge of the bar.
 * @param y Top edge of the bar.
 * @param width Bar width.
 * @param height Bar height.
 * @param radius Corner radius.
 * @param progress Fill percentage, 0-100.
 */
void drawProgressBar(DISPLAY_CLASS &display, ProgressBarStyle style, int16_t x, int16_t y, int16_t width,
                     int16_t height, int16_t radius, int16_t progress);

/// Axis-aligned bounding box of rendered text, as returned by getBounds()/the draw*Text() helpers.
struct Bounds {
    int16_t x;   ///< Left edge, relative to the origin the text was measured/drawn at.
    int16_t y;   ///< Top edge, relative to the origin the text was measured/drawn at.
    uint16_t w;  ///< Width in pixels.
    uint16_t h;  ///< Height in pixels.
};

/**
 * @brief Measure the pixel bounds a string would occupy in a given font, without drawing it.
 * @param display Target display (used only for font metrics).
 * @param text Text to measure.
 * @param font Font to measure with.
 * @return The measured bounds.
 */
Bounds getBounds(DISPLAY_CLASS &display, const char *text, const GFXfont *font);

/**
 * @brief Draw text with its top-left baseline-relative origin at (x, y).
 * @param display Target display.
 * @param text Text to draw.
 * @param x Cursor X.
 * @param y Cursor Y (text baseline, per Adafruit_GFX cursor semantics).
 * @param font Font to draw with.
 * @param color Text color.
 * @return The bounds the text was drawn at.
 */
Bounds drawText(DISPLAY_CLASS &display, const char *text, int16_t x, int16_t y, const GFXfont *font, uint16_t color);

/**
 * @brief Draw text such that its bottom edge sits at y (rather than its baseline).
 * @param display Target display.
 * @param text Text to draw.
 * @param x Left edge X.
 * @param y Y coordinate of the text's bottom edge.
 * @param font Font to draw with.
 * @param color Text color.
 * @return The bounds the text was drawn at.
 */
Bounds drawBottomAlignedText(DISPLAY_CLASS &display, const char *text, int16_t x, int16_t y, const GFXfont *font,
                             uint16_t color);

/**
 * @brief Draw text horizontally and vertically centered on (x, y).
 * @param display Target display.
 * @param text Text to draw.
 * @param x X coordinate to center on.
 * @param y Y coordinate to center on.
 * @param font Font to draw with.
 * @param color Text color.
 * @return The bounds the text was drawn at.
 */
Bounds drawCenteredText(DISPLAY_CLASS &display, const char *text, int16_t x, int16_t y, const GFXfont *font,
                        uint16_t color);

#endif
