/**
 * @file timer_running.cpp
 * @brief Timer::handleRunning() / Timer::drawRunning(): the TimerState::Running / UserInitiatedPause screen.
 */
#include "../timer.h"
#include "../strings.h"
#include "../config.h"

void Timer::handleRunning(volatile const int *encoderCount) {
    if (state == TimerState::UserInitiatedPause && displayBlanked) {
        if (wakeDisplayIfTriggered(encoderCount)) {
            topMenu->setEncoderCount(*encoderCount);  // this input just wakes the display, it's not also a menu move
        }
        // Ignore all other input while blanked; nothing else to do until woken.
        return;
    }

    if (state == TimerState::Running) {
        elapsed = millis() - startTime - totalPausedTime;
    }

    if (state == TimerState::Running && millis() - lastMessageUpdate >= RUNNING_MESSAGE_REFRESH_INTERVAL) {
        lastMessageUpdate = millis();
        messageCache.clearCache(Messages::Preset_RunningMessage);

        needsRedraw = true;
    }

    if (state == TimerState::Running && elapsed >= currentPreset->getDuration()) {
        longestEarnedPauseInShortCycles = max(currentPreset->getLongPauseDuration(), longestEarnedPauseInShortCycles);

        setLedMode(LedMode::ConfirmationFlash);
        state = TimerState::WaitingConfirmStartOfBreak;

        this->confirmationMenu =
            new Menu(display, new MenuItem[1]{MenuItem(messageCache.getMessage(Messages::MenuItem_StartBreak))}, 1);
        this->confirmationMenu->setEncoderCount(*encoderCount);  // Sync the encoder count
        needsFullRedraw = true;
        return;
    }

    // Only trigger redraw once per second to avoid unnecessary updates
    if (millis() - lastRedrawTime >= redrawInterval) {
        needsRedraw = true;
    }

    // Handle menu input
    if (topMenu) {
        if (Button::instance->checkAndClearButtonPress()) {
            // Handle menu selection
            if (topMenu->getSelectedIndex() == 0) {
                if (state == TimerState::Running) {
                    // Pause
                    pause();
                    topMenu->getSelected()->setText(messageCache.getMessage(Messages::MenuItem_Resume));
                } else {
                    // Resume
                    resume();
                    topMenu->getSelected()->setText(messageCache.getMessage(Messages::MenuItem_Pause));
                }

                needsRedraw = true;
            } else if (topMenu->getSelectedIndex() == 1) {
                // Break now
                startBreak();
            } else if (topMenu->getSelectedIndex() == 2) {
                // Cancel
                cancelWorkInterval();
            }

            lastActivityTime = millis();
        }

        if (topMenu->loop(encoderCount)) {
            menuNeedsRedraw = true;
            lastActivityTime = millis();
        }
    }

    if (state == TimerState::UserInitiatedPause && millis() - lastActivityTime >= PAUSE_BLANK_TIMEOUT) {
        blankDisplay(encoderCount);
    }
}

void Timer::drawRunning() {
    display.fillScreen(GxEPD_WHITE);

    if (showSpeechBubble) {
        if (currentPreset->getBackground()) {
            // LPE mode's full-screen background bitmap (see IconProvider).
            display.drawBitmap(0, 0, currentPreset->getBackground(), display.width(), display.height(), GxEPD_BLACK);
        } else {
            // Neutral: drawn rather than stored, ~2 KB (the icon) instead of a 48 KB bitmap. The
            // bubble encloses the message area used below (221,330 to 649,409).
            const ScaledIcon person = icon_person.scaled(128);
            display.drawBitmap(48, display.height() - person.size, person.data, person.size, person.size, GxEPD_BLACK);
            drawSpeechBubble(display, 196, 318, 470, 108);
        }
    }

    drawMenuBar();

    // REMOVE if TESTING PASSES -> unsigned int remainingUnit = 0;
    char buffer[32];  // Increased buffer size to be safe

    const unsigned int remainingMillis = currentPreset->getDuration() - elapsed;
    const unsigned int seconds = remainingMillis / 1000;
    const unsigned int secondsT = seconds % 60;
    const unsigned int minutes = max(seconds / 60, 1u);
    uint16_t roundedSeconds = (seconds + 9) / 10 * 10;

    if (timerStyleOld) {
        if (roundedSeconds >= 60) {
            sprintf(buffer, "%u %s", minutes, messageCache.getMessage(Messages::TimeFormat_Minutes));
        } else {
            if (redrawInterval != REDRAW_INTERVAL_FAST) {
                redrawInterval = REDRAW_INTERVAL_FAST;
                needsRedraw = true;
            }
            sprintf(buffer, "%d %s", roundedSeconds, messageCache.getMessage(Messages::TimeFormat_Seconds));
        }
    } else {
        sprintf(buffer, "%02u:%02u", seconds / 60, secondsT);
    }

    const uint16_t progressBarHeight = 32;

    Bounds boundsMin = getBounds(display, buffer, &LARGE_FONT);

    if (!showSpeechBubble) {
        // Draw text in the center
        drawText(display, buffer, display.width() / 2 - boundsMin.w / 2, display.height() / 2 + boundsMin.h / 2,
                 &LARGE_FONT, GxEPD_BLACK);
        return;
    }

    drawText(display, buffer, display.width() / 2 - boundsMin.w / 2, display.height() / 3 + boundsMin.h / 2,
             &LARGE_FONT, GxEPD_BLACK);

    const unsigned int progress = (elapsed * 100) / currentPreset->getDuration();
    const uint16_t progressBarWidth = display.width() / 2;

    drawProgressBar(display, ProgressBarStyle::Bordered, display.width() / 2 - progressBarWidth / 2,
                    display.height() / 3 + boundsMin.h / 2 + 16, progressBarWidth, progressBarHeight,
                    progressBarHeight / 2, progress);

    drawDebugCrosshair(display, display.width() / 2, display.height() / 2, 48);

    // Message area inside the speech bubble: 221,330 to 649,~425 (see drawSpeechBubble() above).
    const uint16_t messageMinX = 221;
    const uint16_t messageWidth = 649 - messageMinX;
    const uint16_t messageMinY = 330;
    const uint16_t lineHeight = 20;
    const unsigned int maxLines = 4;

    drawDebugCrosshair(display, messageMinX, messageMinY);

    // Wrapped to the bubble's width at draw time, so messages need no hand-placed line breaks.
    bool truncated = false;
    const char *message = getRunningMessage();
    const std::vector<std::string> lines = wrapText(display, message, &SMALL_FONT, messageWidth, maxLines, &truncated);
    if (truncated) {
        ESP_LOGW("TIMER", "drawRunning: message too long for %u lines, cut short: \"%s\"", maxLines, message);
    }
    for (unsigned int i = 0; i < lines.size(); i++) {
        const int16_t baseline = messageMinY + 18 + i * lineHeight;  // + 18: the first line's ascent
        drawText(display, lines[i].c_str(), messageMinX, baseline, &SMALL_FONT, GxEPD_BLACK);
    }
}

const char *Timer::getRunningMessage() {
    if (currentPreset == nullptr) {
        return "No preset selected";
    }

    // Every preset shares one message pool, so presets added to PRESETS (config.h) need nothing
    // here. (This used to match "Emails"/"Coding"/"Focus" by name and show "Unknown preset" for
    // anything else, such as "Workshop".)
    return messageCache.getMessage(Messages::Preset_RunningMessage);
}
