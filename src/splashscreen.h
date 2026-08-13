/**
 * @file splashscreen.h
 * @brief The boot-time "Start / Settings" screen, and the settings sub-screen it leads to.
 *
 * Both loop() and loopSettings() block the caller (they run their own `while (true)` polling
 * loop) until the user makes a choice, rather than being driven from the main Arduino loop().
 */
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include "gfx_utils.h"
#include "icons.h"
#include "icon.h"
#include "timer.h"
#include "checkbox.h"
#include "debug.h"
#include "images.h"
#include <vector>

/// Boot-time screen offering "Start" (begins the Timer) or "Settings" (persisted Checkbox options).
class SplashScreen {
   private:
    DISPLAY_CLASS &display;
    Timer &timer;

    Menu buttons = Menu(display, new MenuItem[2]{MenuItem("Start"), MenuItem(Msgs::SETTINGS)}, 2);

    std::vector<Checkbox> checkboxes = {
        Checkbox(&icon_lpehacker, Msgs::MODE, "lpe", true),
        Checkbox(&icon_lpenote, Msgs::MISCMSG, "msgs", true),
        Checkbox(nullptr, Msgs::RSTDEVICE, "reset"),
    };

    Checkbox *selectedCheckbox;
    int16_t selectedSettingsIndex = 0;
    int16_t lastEncoderCount = 0;

   public:
    /**
     * @brief Construct the splash screen, loading persisted checkbox settings immediately.
     * @param display Shared e-paper display reference.
     * @param timer Timer instance to start() when "Start" is chosen.
     */
    SplashScreen(DISPLAY_CLASS &display, Timer &timer);
    ~SplashScreen();  ///< Destructor.

    /**
     * @brief Declared but not yet implemented anywhere, and currently unused.
     * @param count Unused.
     */
    void setLastEncoderCount(int16_t count);

    /// Draw the "Start / Settings" screen.
    void draw();

    /**
     * @brief Block until the user picks "Start" or "Settings", handling the choice before returning.
     *
     * Picking "Start" calls `timer.start()` and returns; picking "Settings" runs drawSettings()
     * and loopSettings() (which itself blocks) before returning.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     */
    void loop(volatile const int *encoderCount);

    /// Draw the settings screen, listing each persisted Checkbox with the selected one highlighted.
    void drawSettings();

    /**
     * @brief Block, handling settings navigation/toggling until the device restarts.
     *
     * A button press toggles and saves the selected checkbox (or, for the "Reset Device" entry,
     * clears all persisted preferences) and then calls `ESP.restart()` — this function does not
     * return under normal use.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     */
    void loopSettings(volatile const int *encoderCount);
};

#endif
