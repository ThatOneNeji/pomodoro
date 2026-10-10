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
 * @brief Resolves every icon/image choice that differs between the neutral and "LPE" (meme) sets.
 *
 * "LPE mode" is a persisted user setting (the "lpe" preferences key, see Checkbox) that swaps in
 * the meme art. That art, and the setting itself, only exist when ::INCLUDE_MEME_ASSETS is
 * enabled in config.h; otherwise LPE mode is always off and only neutral assets are returned.
 * Nothing outside this class should name a meme asset directly.
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

    /// @param enabled Whether the "LPE" set should be used. Ignored unless ::INCLUDE_MEME_ASSETS.
    void setLpeMode(bool enabled) { lpeModeEnabled = INCLUDE_MEME_ASSETS && enabled; }

    /// @return Whether the alternate ("LPE") icon/background set is currently active.
    bool isLpeModeEnabled() const { return lpeModeEnabled; }

    /// @return The full-screen background image to draw behind a running timer, for the active mode.
    const unsigned char *getTimerRunningBackgroundImage() {
#if INCLUDE_MEME_ASSETS
        if (lpeModeEnabled) {
            return image_bg_lpe_bubble;
        }
#endif
        return image_bg_bubble;
    }

    /**
     * @brief Pick an image for a long break.
     * @return A random full-screen long-break image in LPE mode, or nullptr in the neutral set,
     * in which case the long break shows the same statistics layout as a short break.
     */
    const unsigned char *getLongBreakImage() {
#if INCLUDE_MEME_ASSETS
        if (lpeModeEnabled) {
            const unsigned char *images[] = {image_bg_cat, image_bg_stonks, image_bg_pablo, image_bg_what_a_week};
            return images[random(0, sizeof(images) / sizeof(images[0]))];
        }
#endif
        return nullptr;
    }

    /// @return The icon for the "time's up" confirmation screens, for the active mode.
    Icon *getConfirmationIcon() {
#if INCLUDE_MEME_ASSETS
        if (lpeModeEnabled) {
            return &icon_lpetantrum;
        }
#endif
        return &icon_bell;
    }

    /**
     * @brief Look up the icon for a preset by name, for the active mode.
     * @param name Preset name (see ::PRESETS in config.h). Names without their own icon get
     *             the stopwatch icon.
     * @return Pointer to the matching Icon.
     */
    Icon *getPresetIcon(const char *name) {
#if INCLUDE_MEME_ASSETS
        if (lpeModeEnabled) {
            if (strcmp(name, "Coding") == 0) {
                return &icon_lpehacker;
            } else if (strcmp(name, "Focus") == 0) {
                return &icon_lpethink;
            }
            // No LPE art for other presets: fall through to the neutral icons.
        }
#endif
        if (strcmp(name, "Coding") == 0) {
            return &icon_coding;
        } else if (strcmp(name, "Focus") == 0) {
            return &icon_focus;
        } else if (strcmp(name, "Workshop") == 0) {
            return &icon_workshop;
        }

        return &icon_stopwatch;
    }
};

#endif  // ICON_PROVIDER_H
