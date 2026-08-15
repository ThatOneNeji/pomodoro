/**
 * @file anniversary.h
 * @brief One-off, paged full-screen presentation shown instead of the normal timer UI.
 *
 * Gated behind `ANNIVERSARY_MODE` in config.h (currently disabled). There is no corresponding
 * anniversary.cpp yet, so this class is declared but unimplemented; enabling `ANNIVERSARY_MODE`
 * as-is will fail to link.
 */
#ifndef ANNIVERSARY_H
#define ANNIVERSARY_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include "timer.h"
#include "gfx_utils.h"
#include "icons.h"
#include "icon.h"
#include "menu.h"
#include "debug.h"
#include "button.h"
#include <vector>

/// A paged, full-screen sequence shown once in place of the normal timer UI.
class Anniversary {
   private:
    DISPLAY_CLASS &display;  ///< Shared e-paper display reference.

    Menu buttons = Menu(display, new MenuItem[1]{MenuItem("Weiter")}, 1);  ///< Single "Next" control.

    int page = 0;                      ///< Index of the page currently shown.
    const uint16_t numPages = 17;      ///< Total number of pages in the sequence.
    unsigned long lastRedrawTime = 0;  ///< millis() timestamp of the last screen redraw.

    /**
     * @brief Render the current page's content into the given screen region.
     * @param x Left edge of the drawable area.
     * @param y Top edge of the drawable area.
     * @param w Width of the drawable area.
     * @param h Height of the drawable area.
     */
    void drawPage(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

   public:
    /**
     * @brief Construct the presentation against the given display.
     * @param display Shared e-paper display reference.
     */
    Anniversary(DISPLAY_CLASS &display);
    ~Anniversary();

    /// Draw the current page.
    void draw();

    /// Block and run the paged presentation until it completes (advances on button press).
    void loop();
};

#endif
