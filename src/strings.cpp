/// @file strings.cpp
#include "strings.h"
#include <type_traits>

MessageCache messageCache;

bool MessageCache::isLpeModeEnabled() {
#if INCLUDE_MEME_ASSETS
    return pref_getCheckbox("lpe", false);
#else
    return false;  // LPE mode doesn't exist in this build; see INCLUDE_MEME_ASSETS in config.h.
#endif
}

static const char *randomMessage(const std::vector<const char *> &messages) {
    if (messages.empty())
        return "???";
    return messages[random(0, messages.size())];
}

template<std::size_t N> static const char *randomMessage(const std::array<const char *, N> &messages) {
    if (messages.empty())
        return "???";
    return messages[random(0, messages.size())];
}

/// LPE-mode variant of genericStartBreakMessages. Currently empty; randomMessage() falls back to "???" until populated.
extern const std::vector<const char *> lpeStartBreakMessages = {};
/// LPE-mode additions layered onto Messages::Preset_RunningMessage. Currently empty.
static const std::vector<const char *> lpeRunningMessages = {};

static const char *generateMessage(Messages message) {
    const char *result = "";
    std::vector<const char *> messages;
    messages.insert(messages.end(), Msgs::CHAT_GPT_FACTS.begin(), Msgs::CHAT_GPT_FACTS.end());
    messages.insert(messages.end(), Msgs::GENERIC_PRESET_MESSAGES.begin(), Msgs::GENERIC_PRESET_MESSAGES.end());

    bool lpeModeEnabled = messageCache.isLpeModeEnabled();

    switch (message) {
        case Messages::TimerWaitingForConfirmationStartOfBreak_Header:
            result = Msgs::DONE;
            break;

        case Messages::TimerWaitingForConfirmationEndOfBreak_Header:
            result = Msgs::BREAKOVER;
            break;

        case Messages::Break_PauseText:
            result = Msgs::PAUSE;
            break;

        case Messages::Break_LongPauseText:
            result = Msgs::BREAKOVER;
            break;

        // Menu items
        case Messages::MenuItem_Pause:
            if (lpeModeEnabled) {
                result = Msgs::PAUSE;
            } else {
                result = Msgs::PAUSE;
            }
            break;
        case Messages::MenuItem_Resume:
            if (lpeModeEnabled) {
                result = Msgs::RESUME;
            } else {
                result = Msgs::RESUME;
            }
            break;
        case Messages::MenuItem_BreakNow:
            if (lpeModeEnabled) {
                result = Msgs::BREAKNOW;
            } else {
                result = Msgs::BREAKNOW;
            }
            break;
        case Messages::MenuItem_SkipBreak:
            if (lpeModeEnabled) {
                result = Msgs::SKIPBREAK;
            } else {
                result = Msgs::SKIPBREAK;
            }
            break;
        case Messages::MenuItem_Cancel:
            result = Msgs::STOP;
            break;
        case Messages::MenuItem_BackToPresets:
            result = Msgs::TOSELECT;
            break;
        case Messages::MenuItem_RestartTimer:
            // TODO: Convert these struct
            if (lpeModeEnabled) {
                result = randomMessage({"Noch mal!", "AGAIN!", "Here we go again...", "Do it agane"});
            } else {
                result = randomMessage({"Restart", "Let's go again", "One more time"});
            }
            break;
        case Messages::MenuItem_StartBreak:
            if (lpeModeEnabled) {
                result = randomMessage(lpeStartBreakMessages);
            } else {
                result = randomMessage(Msgs::BREAK_MESSAGES);
            }
            break;

        // Timer states
        case Messages::TimerState_Paused:
            result = Msgs::PAUSED;
            break;

        // Time formats
        case Messages::TimeFormat_Minutes:
            result = "min";
            break;
        case Messages::TimeFormat_Seconds:
            result = "sec";
            break;

        // Running presets: trivia and generic lines, the same for every preset.
        case Messages::Preset_RunningMessage:
            if (lpeModeEnabled) {
                messages.insert(messages.end(), lpeRunningMessages.begin(), lpeRunningMessages.end());
            }
            result = randomMessage(messages);
            break;

        case Messages::Statistics:
            result = Msgs::STATISTICS;
            break;

        case Messages::Statistics_CurrentCycle:
            result = Msgs::CURRENTCYCLE;
            break;

        case Messages::Statistics_CurrentTime:
            result = Msgs::CURRENTTIME;
            break;

        case Messages::Statistics_CurrentBreakTime:
            result = Msgs::CURRENTBREAKTIME;
            break;

        case Messages::Statistics_TotalCycles:
            result = Msgs::TOTALCYCLES;
            break;

        case Messages::Statistics_TotalTime:
            result = Msgs::TOTALTIME;
            break;

        case Messages::Statistics_TotalBreakTime:
            result = Msgs::TOTALBREAKTIME;
            break;

        default:
            result = "???";
            break;
    }

    return result;
}

const char *MessageCache::getMessage(Messages message) {
    auto it = cache.find(message);
    if (it != cache.end()) {
        return it->second;
    }

    const char *result = generateMessage(message);
    cache[message] = result;
    return result;
}

void MessageCache::clearCache(Messages message) { cache.erase(message); }

void MessageCache::clearAllCache() { cache.clear(); }

std::vector<const char *> MessageCache::getMessages() {
    std::vector<const char *> messages;
    // append vectors
    messages.insert(messages.end(), Msgs::GENERIC_PRESET_MESSAGES.begin(), Msgs::GENERIC_PRESET_MESSAGES.end());
    messages.insert(messages.end(), Msgs::BREAK_MESSAGES.begin(), Msgs::BREAK_MESSAGES.end());
    messages.insert(messages.end(), lpeStartBreakMessages.begin(), lpeStartBreakMessages.end());
    messages.insert(messages.end(), lpeRunningMessages.begin(), lpeRunningMessages.end());
    messages.insert(messages.end(), Msgs::CHAT_GPT_FACTS.begin(), Msgs::CHAT_GPT_FACTS.end());

    return messages;
}

// Every message pool must be fully populated: an empty slot is a null pointer that
// randomMessage() can pick, and drawing it crashes. The arrays in src/i18n/ let the compiler
// count their entries (`std::array NAME = {...}`), which already rules out a size that's larger
// than the list; this also catches an explicit nullptr or a size added back by hand.
template<std::size_t N> static constexpr bool allPopulated(const std::array<const char *, N> &messages) {
    for (const char *message : messages) {
        if (message == nullptr) {
            return false;
        }
    }
    return true;
}
static_assert(allPopulated(Msgs::GENERIC_PRESET_MESSAGES), "empty slot in Msgs::GENERIC_PRESET_MESSAGES");
static_assert(allPopulated(Msgs::CHAT_GPT_FACTS), "empty slot in Msgs::CHAT_GPT_FACTS");
static_assert(allPopulated(Msgs::BREAK_MESSAGES), "empty slot in Msgs::BREAK_MESSAGES");
static_assert(allPopulated(Msgs::RESTARTTIMERMSG), "empty slot in Msgs::RESTARTTIMERMSG");
static_assert(allPopulated(Msgs::RESTARTTIMERMSGLPE), "empty slot in Msgs::RESTARTTIMERMSGLPE");
