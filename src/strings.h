/**
 * @file strings.h
 * @brief Resolves ::Messages keys to localized display strings, selecting the active language at compile time.
 *
 * The active language is chosen by which header under i18n/ is included below, controlled by the
 * `LANG_AF`/`LANG_DE` macros in config.h (English is the default). Each language header defines
 * the same `Msgs` struct API (see i18n/en.h); ::MessageCache::getMessage() maps a ::Messages key
 * onto the right `Msgs` value (and, for messages with several variants, picks one at random).
 */
#ifndef STRINGS_H
#define STRINGS_H

#include <Arduino.h>
#include <math.h>     // for float_t, double_t
#include <WString.h>  // for String
#include <Preferences.h>
#include "preferences_manager.h"
#include <vector>
#include <map>

#if defined(LANG_AF)
// Uncomment in config.h for Afrikaans
#include "i18n/af.h"
#elif defined(LANG_DE)
// Uncomment in config.h for German
#include "i18n/de.h"
#else
// Using the default language
#include "i18n/en.h"
#endif

/// Identifies a piece of localized UI text, resolved to an actual string via MessageCache::getMessage().
enum class Messages {
    TimerWaitingForConfirmationStartOfBreak_Header,
    TimerWaitingForConfirmationEndOfBreak_Header,

    Break_PauseText,
    Break_LongPauseText,

    // Menu items
    MenuItem_Pause,
    MenuItem_Resume,
    MenuItem_BreakNow,
    MenuItem_SkipBreak,
    MenuItem_Cancel,
    MenuItem_BackToPresets,
    MenuItem_RestartTimer,
    MenuItem_StartBreak,

    // Timer states
    TimerState_Paused,

    // Time formats
    TimeFormat_Minutes,
    TimeFormat_Seconds,

    // Preset specific messages
    Preset_Email_Message,
    Preset_Coding_Message,
    Preset_Focus_Message,

    Statistics,
    Statistics_CurrentCycle,
    Statistics_CurrentTime,
    Statistics_CurrentBreakTime,
    Statistics_TotalCycles,
    Statistics_TotalTime,
    Statistics_TotalBreakTime,
};

/**
 * @brief Resolves Messages keys to localized strings, caching each resolved value.
 *
 * Messages with several candidate strings (e.g. GENERIC_PRESET_MESSAGES) are resolved to one at
 * random on first lookup, then cached — so repeated calls return the same string until
 * clearCache()/clearAllCache() forces a fresh pick.
 */
class MessageCache {
   private:
    std::map<Messages, const char *> cache;

   public:
    MessageCache() {}  ///< Constructs an empty cache.

    /// @return Whether the persisted "LPE mode" setting is enabled (see the "lpe" preferences key).
    bool isLpeModeEnabled();

    /**
     * @brief Resolve a message key to its localized string, computing and caching it on first use.
     * @param message The message to resolve.
     * @return The resolved string.
     */
    const char *getMessage(Messages message);

    /**
     * @brief Evict a single cached message, so the next getMessage() call for it re-resolves.
     * @param message The message to evict.
     */
    void clearCache(Messages message);

    /// Evict every cached message.
    void clearAllCache();

    /// @return Every candidate message string across all message lists, concatenated (used by the STRINGS_TEST debug
    /// mode).
    std::vector<const char *> getMessages();
};

/// Shared message cache instance used throughout the UI.
extern MessageCache messageCache;

#endif
