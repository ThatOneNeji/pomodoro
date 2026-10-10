/// @file settings_screen.cpp
#include "settings_screen.h"
#include "button.h"
#include "gfx_utils.h"
#include "icons.h"
#include "esp_log.h"
#include <Preferences.h>

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "SETTINGS";

SettingsScreen::SettingsScreen(DISPLAY_CLASS &display) : display(display) {
    for (auto &checkbox : checkboxes) {
        checkbox.load();
    }
}

void SettingsScreen::draw() {
    const int16_t margin = 16;
    const unsigned int rows = checkboxes.size() + 1;  // + Back
    const int16_t rowHeight = 96;
    const int16_t gap = rows <= 3 ? 32 : 16;  // 4 rows (with INCLUDE_MEME_ASSETS) need a tighter gap
    const int16_t width = display.width() - 2 * margin;

    display.fillScreen(GxEPD_WHITE);

    // By reference: Checkbox's destructor saves, so drawing a copy rewrote every setting each redraw.
    for (unsigned int i = 0; i < checkboxes.size(); i++) {
        checkboxes[i].draw(display, margin, margin + i * (rowHeight + gap), width, rowHeight, i == selectedIndex);
    }

    // Back row, laid out like a checkbox row but with an arrow and no check box.
    const int16_t y = margin + checkboxes.size() * (rowHeight + gap);
    const int16_t padding = 12;
    display.drawRoundRect(margin, y, width, rowHeight, 10, GxEPD_BLACK);
    if (selectedIndex == checkboxes.size()) {
        drawPatternInRoundedArea(display, margin, y, width, rowHeight, 10, Pattern::SparseDots);
    }
    const ScaledIcon arrow = icon_back.scaled(64);
    display.drawBitmap(margin + padding, y + padding, arrow.data, arrow.size, arrow.size, GxEPD_BLACK);
    const Bounds bounds = getBounds(display, Msgs::BACK, &MAIN_FONT);
    drawText(display, Msgs::BACK, margin + padding + arrow.size + padding, y + rowHeight / 2 + bounds.h / 2, &MAIN_FONT,
             GxEPD_BLACK);
}

void SettingsScreen::run(volatile const int *encoderCount) {
    const unsigned int rows = checkboxes.size() + 1;
    selectedIndex = 0;
    draw();
    display.display(false);

    int lastEncoderCount = *encoderCount;
    while (true) {
        if (*encoderCount != lastEncoderCount) {
            selectedIndex =
                *encoderCount > lastEncoderCount ? (selectedIndex + 1) % rows : (selectedIndex + rows - 1) % rows;
            lastEncoderCount = *encoderCount;
            draw();
            display.display(true);
        }

        if (Button::instance && Button::instance->checkAndClearButtonPress()) {
            if (selectedIndex == checkboxes.size()) {
                ESP_LOGI(TAG, "run: back");
                return;
            }

            Checkbox &selected = checkboxes[selectedIndex];
            if (strcmp(selected.getName(), Msgs::RSTDEVICE) == 0) {
                ESP_LOGW(TAG, "run: resetting all preferences");
                Preferences preferences;
                preferences.begin("pomodoro", false);
                preferences.clear();
                preferences.end();

                display.fillScreen(GxEPD_BLACK);
                display.display();
                delay(5000);
                ESP.restart();
            }

            // Toggle, save and restart, so everything picks the new value up from scratch.
            selected.toggle();
            selected.save();
            draw();
            display.display(true);
            delay(1000);
            ESP.restart();
        }

        delay(10);
    }
}
