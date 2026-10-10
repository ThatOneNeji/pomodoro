/**
 * @file GxEPD2_BW.h
 * @brief Host-test stand-in for the GxEPD2 e-paper display: draws nothing, counts refreshes.
 *
 * Text is measured as a fixed fake::GLYPH_WIDTH per character, so text layout (e.g. wrapText())
 * can be tested with predictable numbers.
 */
#pragma once

#include <cstring>

#include "Adafruit_GFX.h"
#include "WString.h"

#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF

namespace fake {
inline constexpr uint16_t GLYPH_WIDTH = 10;   ///< Width of every character, in pixels.
inline constexpr uint16_t GLYPH_HEIGHT = 20;  ///< Height of every line of text, in pixels.
}  // namespace fake

/// The panel type named in config.h's DISPLAY_CLASS; only its name matters here.
struct GxEPD2_426_GDEQ0426T82 {};

template<typename Driver, int PageHeight> class GxEPD2_BW {
   public:
    int fullRefreshes = 0;     ///< display(false) calls.
    int partialRefreshes = 0;  ///< display(true) and displayWindow() calls.
    bool hibernating = false;  ///< Set by hibernate(), cleared by init().

    void init(unsigned long, bool, int, bool) { hibernating = false; }
    void hibernate() { hibernating = true; }
    void setRotation(uint8_t) {}
    int16_t width() const { return 800; }
    int16_t height() const { return 480; }

    void firstPage() {}
    void display(bool partial) { partial ? partialRefreshes++ : fullRefreshes++; }
    void displayWindow(int16_t, int16_t, int16_t, int16_t) { partialRefreshes++; }
    void clearScreen() {}
    void fillScreen(uint16_t) {}

    void drawPixel(int16_t, int16_t, uint16_t) {}
    void drawLine(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawFastHLine(int16_t, int16_t, int16_t, uint16_t) {}
    void drawFastVLine(int16_t, int16_t, int16_t, uint16_t) {}
    void drawRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void fillRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawRoundRect(int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void fillRoundRect(int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawCircle(int16_t, int16_t, int16_t, uint16_t) {}
    void fillTriangle(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawBitmap(int16_t, int16_t, const uint8_t *, int16_t, int16_t, uint16_t) {}

    void setFont(const GFXfont *) {}
    void setTextSize(uint8_t) {}
    void setTextColor(uint16_t) {}
    void setCursor(int16_t, int16_t) {}
    void print(const char *) {}
    void print(const String &) {}

    void getTextBounds(const char *text, int16_t, int16_t, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
        *x1 = 0;
        *y1 = -static_cast<int16_t>(fake::GLYPH_HEIGHT);
        *w = std::strlen(text) * fake::GLYPH_WIDTH;
        *h = fake::GLYPH_HEIGHT;
    }
};
