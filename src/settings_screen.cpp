/// @file settings_screen.cpp
#include "settings_screen.h"
#include "button.h"
#include "gfx_utils.h"
#include "icons.h"
#include "power.h"
#include "esp_log.h"
#include <Preferences.h>

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "SETTINGS";

/// Draw an action button in the last row: an icon and a label in a rounded box, dotted when selected.
static void drawAction(DISPLAY_CLASS &display, int16_t x, int16_t y, int16_t w, int16_t h, Icon &icon,
                       const char *label, bool selected) {
    const int16_t padding = 12;
    display.drawRoundRect(x, y, w, h, 10, GxEPD_BLACK);
    if (selected) {
        drawPatternInRoundedArea(display, x, y, w, h, 10, Pattern::SparseDots);
    }
    const ScaledIcon scaled = icon.scaled(64);
    display.drawBitmap(x + padding, y + (h - scaled.size) / 2, scaled.data, scaled.size, scaled.size, GxEPD_BLACK);
    const Bounds bounds = getBounds(display, label, &MAIN_FONT);
    drawText(display, label, x + padding + scaled.size + padding, y + h / 2 + bounds.h / 2, &MAIN_FONT, GxEPD_BLACK);
}

SettingsScreen::SettingsScreen(DISPLAY_CLASS &display) : display(display) {
    for (auto &checkbox : checkboxes) {
        checkbox.load();
    }
}

void SettingsScreen::draw() {
    const int16_t margin = 16;
    const unsigned int rows = checkboxes.size() + 1;  // + the Back / Shut down row
    const int16_t rowHeight = 96;
    const int16_t gap = rows <= 3 ? 32 : 16;  // 4 rows (with INCLUDE_MEME_ASSETS) need a tighter gap
    const int16_t width = display.width() - 2 * margin;

    display.fillScreen(GxEPD_WHITE);

    // By reference: Checkbox's destructor saves, so drawing a copy rewrote every setting each redraw.
    for (unsigned int i = 0; i < checkboxes.size(); i++) {
        checkboxes[i].draw(display, margin, margin + i * (rowHeight + gap), width, rowHeight, i == selectedIndex);
    }

    // Last row: Back and Shut down side by side, so the screen needs no extra row for them.
    const int16_t y = margin + checkboxes.size() * (rowHeight + gap);
    const int16_t half = (width - gap) / 2;
    drawAction(display, margin, y, half, rowHeight, icon_back, Msgs::BACK, selectedIndex == backIndex());
    drawAction(display, margin + half + gap, y, half, rowHeight, icon_power, Msgs::SHUTDOWN,
               selectedIndex == shutDownIndex());
}

void SettingsScreen::run(volatile const int *encoderCount) {
    const unsigned int positions = shutDownIndex() + 1;  // the checkboxes, then Back, then Shut down
    selectedIndex = 0;
    draw();
    display.display(false);

    int lastEncoderCount = *encoderCount;
    while (true) {
        if (*encoderCount != lastEncoderCount) {
            selectedIndex = *encoderCount > lastEncoderCount ? (selectedIndex + 1) % positions
                                                             : (selectedIndex + positions - 1) % positions;
            lastEncoderCount = *encoderCount;
            draw();
            display.display(true);
        }

        if (Button::instance && Button::instance->checkAndClearButtonPress()) {
            if (selectedIndex == backIndex()) {
                ESP_LOGI(TAG, "run: back");
                return;
            }
            if (selectedIndex == shutDownIndex()) {
                shutDown(display);  // doesn't return
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
