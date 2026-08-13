/**
 * @file menu.h
 * @brief A rotary-encoder-driven selectable list of labeled items (used for the top menu bar and confirmation prompts).
 */
#ifndef MENU_H
#define MENU_H

#include "defs.h"
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include "icon.h"
#include "debug.h"

/// Layout orientation for a menu. Currently unused by any drawing code.
enum class DrawStyle { Vertical, Horizontal };

/// A single labeled, optionally-iconed entry in a Menu.
class MenuItem {
   private:
    const char *text;
    Icon *icon;

   public:
    /**
     * @brief Construct a menu item.
     * @param text Label text.
     * @param icon Optional icon (nullptr for text-only items).
     */
    explicit MenuItem(const char *text, Icon *icon = nullptr);
    ~MenuItem();  ///< Destructor.

    /// @return The label text.
    const char *getText();

    /// @param text New label text.
    void setText(const char *text);

    /// @return The item's icon, or nullptr if it has none.
    Icon *getIcon();
};

/**
 * @brief A fixed-size list of MenuItem entries with encoder-driven selection.
 *
 * Selection is advanced by comparing the shared encoder count against the count last seen
 * (see loop()), rather than the menu owning any input hardware itself.
 */
class Menu {
   private:
    DISPLAY_CLASS &display;  // Update display type
    MenuItem *items;
    int itemCount;
    int selectedIndex;

    int lastEncoderCount = 0;

   public:
    /**
     * @brief Construct a menu over an existing array of items.
     * @param display Shared e-paper display reference (not drawn to by Menu itself; kept for callers/subclasses).
     * @param items Array of menu items; ownership stays with the caller.
     * @param itemCount Number of entries in @p items.
     */
    Menu(DISPLAY_CLASS &display, MenuItem *items, int itemCount);  // Update constructor
    ~Menu();                                                       ///< Destructor.

    /// @return The currently selected item.
    MenuItem *getSelected();

    /// @return The underlying items array.
    MenuItem *getItems();

    /// @return The index of the currently selected item.
    int getSelectedIndex();

    /// @return The number of items in the menu.
    int getItemCount();

    /// @param index Index to select.
    void setSelectedIndex(int index);

    /**
     * @brief Resynchronize the baseline encoder count without changing selection.
     * @param encoderCount The encoder count to treat as "no movement yet".
     */
    void setEncoderCount(int encoderCount);

    /// Select the next item, wrapping around at the end.
    void next();

    /// Select the previous item, wrapping around at the start.
    void previous();

    /**
     * @brief Advance selection if the shared encoder count has changed since the last call.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     * @return true if the encoder moved and selection changed, false otherwise.
     */
    bool loop(volatile const int *encoderCount);
};

#endif  // MENU_H
