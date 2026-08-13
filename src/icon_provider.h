/**
 * @file icon_provider.h
 * @brief Singleton that picks the right icon/background variant for the active mode ("LPE" vs. normal).
 */
#ifndef ICON_PROVIDER_H
#define ICON_PROVIDER_H

#include "icon.h"
#include "icons.h"
#include "images.h"

/**
 * @brief Resolves preset icons and backgrounds, swapping in an alternate ("LPE") icon set when enabled.
 *
 * "LPE mode" is a persisted user setting (see Checkbox and the "lpe" preferences key) that swaps
 * the normal icon/background set for a themed alternate one; this class is the single place that
 * knows which variant to hand out for a given preset name.
 */
class IconProvider {
   private:
    static IconProvider *instance;  ///< Lazily-created singleton instance.
    bool lpeModeEnabled = false;    ///< Whether the alternate ("LPE") icon/background set is active.

   public:
    /// @return The singleton instance, creating it on first call.
    static IconProvider *getInstance() {
        if (!instance) {
            instance = new IconProvider();
        }
        return instance;
    }

    /// @param enabled Whether the alternate ("LPE") icon/background set should be used.
    void setLpeMode(bool enabled) { lpeModeEnabled = enabled; }

    /// @return Whether the alternate ("LPE") icon/background set is currently active.
    bool isLpeModeEnabled() const { return lpeModeEnabled; }

    /// @return The full-screen background image to draw behind a running timer, for the active mode.
    const unsigned char *getTimerRunningBackgroundImage() {
        if (lpeModeEnabled) {
            return image_bg_lpe_bubble;
        } else {
            return image_bg_bubble;
        }
    }

    /**
     * @brief Look up the icon for a preset by name, for the active mode.
     * @param name Preset name ("Coding", "Emails", or "Focus"); any other value falls back to
     *             the "Coding" (LPE) icon.
     * @return Pointer to the matching Icon.
     */
    Icon *getPresetIcon(const char *name) {
        if (strcmp(name, "Coding") == 0) {
            if (lpeModeEnabled) {
                return &icon_lpehacker;
            } else {
                return &icon_coding;
            }
        } else if (strcmp(name, "Emails") == 0) {
            if (lpeModeEnabled) {
                return &icon_lpetantrum;
            } else {
                return &icon_email;
            }
        } else if (strcmp(name, "Focus") == 0) {
            if (lpeModeEnabled) {
                return &icon_lpethink;
            } else {
                return &icon_focus;
            }
        }

        return &icon_lpehacker;
    }
};

#endif  // ICON_PROVIDER_H