/**
 * @file splashscreen.h
 * @brief The boot-time splash screen: the "Pomodoro" logo, shown for ::SPLASH_DURATION_MS.
 */
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <Arduino.h>
#include "config.h"

/**
 * @brief Shows the logo at boot, then hands over to preset selection.
 *
 * It has no menu: Status and Settings live on the preset selection screen. Moving straight on
 * also means the idle display blanking (which only runs once the timer has taken over) works
 * from boot, rather than after someone has pressed Start.
 */
class SplashScreen {
   private:
    DISPLAY_CLASS &display;

   public:
    /// @param display Shared e-paper display reference.
    explicit SplashScreen(DISPLAY_CLASS &display);

    /// Draw the logo, wait ::SPLASH_DURATION_MS, and discard any button press made meanwhile, so
    /// it can't carry over and start a preset.
    void show();
};

#endif
