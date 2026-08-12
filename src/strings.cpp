#include "strings.h"
#include <type_traits>

MessageCache messageCache;

bool MessageCache::isLpeModeEnabled() { return pref_getCheckbox("lpe", false); }

static const char *randomMessage(const std::vector<const char *> &messages) {
    if (messages.empty())
        return "???";
    return messages[random(0, messages.size())];
}

static const char *randomMessageV2(const std::array<const char *, 4> &messages) {
    if (messages.empty())
        return "???";
    return messages[random(0, messages.size())];
}

extern const std::vector<const char *> lpeStartBreakMessages = {};
static const std::vector<const char *> lpeEmailPresetMessages = {};
static const std::vector<const char *> lpeCodingPresetMessages = {};

static const char *generateMessage(Messages message) {
    const char *result = "";
    std::vector<const char *> messages;
    messages.insert(messages.end(), chatGptFacts.begin(), chatGptFacts.end());
    messages.insert(messages.end(), genericPresetMessages.begin(), genericPresetMessages.end());

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
                result = randomMessage(genericStartBreakMessages);
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

        // Preset specific
        case Messages::Preset_Email_Message:
            if (lpeModeEnabled) {
                messages.insert(messages.end(), lpeEmailPresetMessages.begin(), lpeEmailPresetMessages.end());
            }
            result = randomMessage(messages);
            break;

        case Messages::Preset_Coding_Message:
            if (lpeModeEnabled) {
                messages.insert(messages.end(), lpeCodingPresetMessages.begin(), lpeCodingPresetMessages.end());
            }
            result = randomMessage(messages);
            break;

        case Messages::Preset_Focus_Message:
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
    messages.insert(messages.end(), genericPresetMessages.begin(), genericPresetMessages.end());
    messages.insert(messages.end(), genericStartBreakMessages.begin(), genericStartBreakMessages.end());
    messages.insert(messages.end(), lpeStartBreakMessages.begin(), lpeStartBreakMessages.end());
    messages.insert(messages.end(), lpeEmailPresetMessages.begin(), lpeEmailPresetMessages.end());
    messages.insert(messages.end(), lpeCodingPresetMessages.begin(), lpeCodingPresetMessages.end());
    messages.insert(messages.end(), chatGptFacts.begin(), chatGptFacts.end());

    return messages;
}
