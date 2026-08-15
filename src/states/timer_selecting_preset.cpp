/**
 * @file timer_selecting_preset.cpp
 * @brief Timer::handleSelectingPreset() / Timer::drawPresetSelection(): the TimerState::SelectingPreset screen.
 */
#include "../timer.h"
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "TIMER";

#define THRESHOLD 5  ///< Unused.

void Timer::handleSelectingPreset(volatile const int *encoderCount) {
    if (displayBlanked) {
        if (wakeDisplayIfTriggered(encoderCount)) {
            lastEncoderCount = *encoderCount;  // this input just wakes the display, it's not also a preset move
        }
        // Ignore all other input while blanked; nothing else to do until woken.
        return;
    }

    if (Button::instance->checkAndClearButtonPress()) {
        ESP_LOGI(TAG, "handleSelectingPreset: starting with preset %u", presetIndex);
        start();
        topMenu->setEncoderCount(*encoderCount);
        needsFullRedraw = true;
        lastActivityTime = millis();
    }

    if (*encoderCount != lastEncoderCount) {
        int change = *encoderCount - lastEncoderCount;
        ESP_LOGD(TAG, "handleSelectingPreset: encoder delta %d", change);

        if (change < 0) {
            previousPreset();
        } else {
            nextPreset();
        }

        ESP_LOGI(TAG, "handleSelectingPreset: selected preset %u (%s)", presetIndex, currentPreset->getName());
        needsRedraw = true;
        lastEncoderCount = *encoderCount;
        lastActivityTime = millis();
    }

    if (millis() - lastActivityTime >= IDLE_BLANK_TIMEOUT) {
        blankDisplay(encoderCount);
    }
}

void Timer::drawPresetSelection() {
    const unsigned int padding = 12;
    const unsigned int paddingBetweenBoxes = 18;
    const unsigned int paddingInsideBox = 10;
    const unsigned int totalWidth = display.width() - (padding * 2);
    const unsigned int boxWidth = (totalWidth + paddingBetweenBoxes) / presets.size() - paddingBetweenBoxes;

    // REMOVE if TESTING PASSES -> unsigned int yOffset = padding;

    display.fillScreen(GxEPD_WHITE);

    for (int i = 0; i < presets.size(); i++) {
        auto &preset = presets[i];
        const unsigned int xOffset = padding + (i * (boxWidth + paddingBetweenBoxes));
        const unsigned int hCenter = xOffset + (boxWidth / 2);

        ScaledIcon icon = preset.getIcon()->scaled(192);
        const unsigned int iconSize = icon.size;

#ifdef DEBUG
        display.drawLine(hCenter, 0, hCenter, display.height(), GxEPD_BLACK);
#endif

        unsigned int yOffset = padding;

        display.drawRoundRect(xOffset, padding, boxWidth, display.height() - (padding * 2), 10, GxEPD_BLACK);

        if (currentPreset == &preset) {
            display.drawRoundRect(xOffset + 1, padding + 1, boxWidth - 2, display.height() - (padding * 2) - 2, 10,
                                  GxEPD_BLACK);
            drawPatternInRoundedArea(display, xOffset + 2, padding + 2, boxWidth - 4,
                                     display.height() - (padding * 2) - 4, 10, Pattern::VerySparseDots);
        }

        yOffset += paddingInsideBox;
        display.drawBitmap(hCenter - iconSize / 2, yOffset, icon.data, iconSize, iconSize, GxEPD_BLACK);

#ifdef DEBUG
        display.fillRect(hCenter - iconSize / 2, yOffset, iconSize, iconSize, GxEPD_BLACK);
#endif

        yOffset += iconSize + paddingInsideBox + 48;

        const Bounds bounds = getBounds(display, preset.getName(), &MAIN_FONT);
        drawText(display, preset.getName(), hCenter - bounds.w / 2, yOffset, &MAIN_FONT, GxEPD_BLACK);

        yOffset += 24 + 48;
        const unsigned int minutes = preset.getDuration() / 1000 / 60;

        // put the text on the right side of the box, on top of the progress bar
        {
            char buffer[12];  // max unsigned int digits (10) + null terminator, plus margin
            snprintf(buffer, sizeof(buffer), "%u", minutes);
            Bounds minutesBounds = getBounds(display, buffer, &SECONDARY_FONT);
            Bounds boundsMin = getBounds(display, "min", &SUB_FONT);
            const int16_t paddingBetweenText = 4;
            const int16_t endOfBoxContent = xOffset + boxWidth - paddingInsideBox;

            yOffset += 24;

            drawText(display, buffer, endOfBoxContent - minutesBounds.w - paddingBetweenText - boundsMin.w, yOffset,
                     &SECONDARY_FONT, GxEPD_BLACK);
            drawText(display, "min", endOfBoxContent - boundsMin.w, yOffset, &SUB_FONT, GxEPD_BLACK);

            yOffset += (max(minutesBounds.h, boundsMin.h) / 2) + 4;

            // display.drawFastHLine(xOffset + paddingInsideBox, yOffset, boxWidth - paddingBetweenBoxes -
            // paddingInsideBox * 2, GxEPD_BLACK);
            drawPattern(display, Pattern::Dots, xOffset + paddingInsideBox + 4, yOffset,
                        boxWidth - paddingBetweenBoxes - paddingInsideBox * 2 - 8, 2);

            yOffset += 8;

            // draw the pause duration
            const unsigned int pauseMinutes = preset.getPauseDuration() / 1000 / 60;
            char pauseBuffer[12];  // max unsigned int digits (10) + null terminator, plus margin
            snprintf(pauseBuffer, sizeof(pauseBuffer), "%u", pauseMinutes);
            Bounds pauseBounds = getBounds(display, pauseBuffer, &SECONDARY_FONT);
            Bounds pauseBoundsMin = getBounds(display, "min", &SUB_FONT);

            const auto breakIcon = icon_coffee.scaled(48);
            const unsigned int pauseIconSize = breakIcon.size;
            const unsigned int pauseIconX = xOffset + paddingInsideBox;

            display.drawBitmap(pauseIconX, yOffset, breakIcon.data, pauseIconSize, pauseIconSize, GxEPD_BLACK);

            drawText(display, pauseBuffer, endOfBoxContent - pauseBounds.w - paddingBetweenText - pauseBoundsMin.w,
                     yOffset + pauseIconSize - 8, &SECONDARY_FONT, GxEPD_BLACK);
            drawText(display, "min", endOfBoxContent - pauseBoundsMin.w, yOffset + pauseIconSize - 8, &SUB_FONT,
                     GxEPD_BLACK);
        }
    }
}
