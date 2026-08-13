/**
 * @file checkbox.h
 * @brief A labeled, persisted toggle drawn as a selectable row (used for settings screens).
 */
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <Arduino.h>
#include <Preferences.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include "icons.h"
#include "icon.h"
#include "gfx_utils.h"
#include "debug.h"
#include "defs.h"

/**
 * @brief A boolean setting with an icon and label, persisted via Preferences under ::key.
 *
 * State is loaded from Preferences on construction and written back on destruction; call
 * save() explicitly to persist earlier, e.g. right after the user toggles it.
 */
class Checkbox {
   private:
    Icon *icon;         ///< Icon drawn to the left of the label.
    const char *name;   ///< Label text drawn next to the icon.
    const char *key;    ///< Preferences key this checkbox's value is stored under.
    bool checked;       ///< Current (in-memory) checked state.
    bool defaultValue;  ///< Value used the first time ::key has never been stored.

    Preferences preferences;

   public:
    /**
     * @brief Construct a checkbox and immediately load() its persisted value.
     * @param icon Icon drawn to the left of the label (may be nullptr).
     * @param name Label text.
     * @param key Preferences key this checkbox's value is stored under.
     * @param defaultValue Value to use when @p key has no stored value yet.
     */
    Checkbox(Icon *icon, const char *name, const char *key, bool defaultValue = false);

    /// Persists the current state via save() before the object is destroyed.
    ~Checkbox();

    /// @return The icon drawn next to the label.
    Icon *getIcon();

    /// @return The label text.
    const char *getName();

    /// @return The current (in-memory) checked state.
    bool isChecked();

    /// Flip the in-memory checked state. Does not persist; call save() to write it out.
    void toggle();

    /// Load the checked state from Preferences (falling back to ::defaultValue).
    void load();

    /// Persist the current checked state to Preferences.
    void save();

    /**
     * @brief Draw this checkbox as a rounded, bordered row.
     * @param display Target display.
     * @param x Left edge of the row.
     * @param y Top edge of the row.
     * @param w Row width.
     * @param h Row height.
     * @param selected Whether this row is the currently highlighted/selected one.
     */
    void draw(DISPLAY_CLASS &display, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool selected);
};

#endif