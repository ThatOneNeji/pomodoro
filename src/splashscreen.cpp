/// @file splashscreen.cpp
#include "splashscreen.h"
#include "button.h"
#include "gfx_utils.h"
#include "icons.h"

SplashScreen::SplashScreen(DISPLAY_CLASS &display) : display(display) {}

void SplashScreen::show() {
    display.fillScreen(GxEPD_WHITE);

    // Drawn in code rather than stored as a full-screen bitmap: costs ~4.6 KB (the icon) instead of 48 KB.
    const ScaledIcon logo = icon_stopwatch.scaled(192);
    const int16_t logoY = 90;
    display.drawBitmap(display.width() / 2 - logo.size / 2, logoY, logo.data, logo.size, logo.size, GxEPD_BLACK);
    drawCenteredText(display, "Pomodoro", display.width() / 2, logoY + logo.size + 60, &TITLE_FONT, GxEPD_BLACK);
    display.display(false);

    delay(SPLASH_DURATION_MS);

    if (Button::instance) {
        Button::instance->checkAndClearButtonPress();
    }
}
