/**
 * @file settings_screen.h
 * @brief The Settings screen: persisted on/off options, "Reset Device", and Back.
 */
#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include <Arduino.h>
#include <vector>
#include "config.h"
#include "checkbox.h"
#include "messages.h"

/**
 * @brief Lists the persisted Checkbox options; the knob moves between rows and a press acts.
 *
 * Toggling an option saves it and restarts the device, so every part of the firmware picks it up
 * from scratch. "Reset Device" clears all preferences, then restarts. "Back" returns without
 * changing anything (reached from the preset selection screen, an accidental visit must not
 * force a restart).
 */
class SettingsScreen {
   private:
    DISPLAY_CLASS &display;

    std::vector<Checkbox> checkboxes = {
#if INCLUDE_MEME_ASSETS
        Checkbox(&icon_lpehacker, Msgs::MODE, "lpe", false),
#endif
        Checkbox(&icon_messages, Msgs::MISCMSG, "msgs", true),
        Checkbox(nullptr, Msgs::RSTDEVICE, "reset"),
    };

    /// 0..checkboxes.size()-1 selects a checkbox, then backIndex() and shutDownIndex().
    unsigned int selectedIndex = 0;

    /// @return The ::selectedIndex of the Back button (last row, left).
    unsigned int backIndex() const { return checkboxes.size(); }
    /// @return The ::selectedIndex of the Shut down button (last row, right).
    unsigned int shutDownIndex() const { return checkboxes.size() + 1; }

    /// Draw every row, with the selected one highlighted, into the display buffer.
    void draw();

   public:
    /// @param display Shared e-paper display reference. Loads the persisted values immediately.
    explicit SettingsScreen(DISPLAY_CLASS &display);

    /**
     * @brief Show Settings until Back is chosen. Toggling an option or resetting restarts the
     * device, and Shut down switches it off (see power.h), so those never return.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     */
    void run(volatile const int *encoderCount);
};

#endif  // SETTINGS_SCREEN_H
