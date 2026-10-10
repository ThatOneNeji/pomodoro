/// @file status_screen.cpp
#include "status_screen.h"
#include "button.h"
#include "gfx_utils.h"
#include "mqtt_manager.h"
#include "statistics.h"
#include "esp_log.h"
#include <vector>

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "STATUS";

namespace {

/// One "label ........ value" line on a Status page.
struct Row {
    String label;
    String value;
};

/// @return "1h 05m"-style uptime; minutes only, so it doesn't force a redraw every second.
String formatUptime(unsigned long seconds) { return formatDuration(seconds / 60); }

String yesNo(bool value, const char *no) { return value ? String("Yes") : String(no); }

const char *const PAGE_TITLES[] = {"Network", "Time & MQTT", "Device", "Firmware", "Storage & stats"};
const unsigned int PAGE_COUNT = sizeof(PAGE_TITLES) / sizeof(PAGE_TITLES[0]);

std::vector<Row> rowsFor(unsigned int page, const SystemStatus &s) {
    switch (page) {
        case 0:
            return {
                {"WiFi", s.wifiConnected ? String("Connected") : "Connecting to " + s.ssid + "..."},
                {"IP address", s.wifiConnected ? s.ip : String("-")},
                {"Web page", s.wifiConnected ? "http://" + s.ip + "/" : String("-")},
                {"Network", s.ssid},
                {"Signal", s.wifiConnected ? String(s.rssi) + " dBm" : String("-")},
                {"MAC address", s.mac},
            };
        case 1:
            return {
                {"Time synced", yesNo(s.timeSynced, "Not yet")},
                // Minutes only, like uptime, so the clock doesn't force a redraw every second.
                {"Time", s.timeSynced ? s.currentTime.substring(0, 16) : String("-")},
                {"Timezone", s.sntpTimezone},
                {"Time server", s.sntpServer},
                {"MQTT broker",
                 s.mqttEnabled ? String(s.mqttBrokerAddress) + ":" + String(s.mqttBrokerPort) : String("Disabled")},
                {"MQTT", s.mqttEnabled ? yesNo(s.mqttConnected, "Not connected") : String("Disabled")},
            };
        case 2:
            return {
                {"Chip", s.chipModel + " rev " + String(s.chipRevision)},
                {"CPU", String(s.cpuMhz) + " MHz"},
                {"Uptime", formatUptime(s.uptimeSeconds)},
                {"Free memory", formatBytes(s.freeHeapBytes)},
            };
        case 3: {
            const unsigned int percent =
                s.appPartitionBytes ? (unsigned int) (100ULL * s.firmwareBytes / s.appPartitionBytes) : 0;
            return {
                {"Version", s.firmwareVersion},
                {"Built", s.firmwareBuildTime},
                {"Firmware size", formatBytes(s.firmwareBytes) + " of " + formatBytes(s.appPartitionBytes) + " (" +
                                      String(percent) + "%)"},
            };
        }
        default:
            return {
                {"Web files", s.fsTotalBytes ? formatBytes(s.fsUsedBytes) + " of " + formatBytes(s.fsTotalBytes)
                                             : String("Not mounted")},
                {"Total cycles", String(s.totalCycles)},
                {"Total work time", formatDuration(s.totalWorkMinutes)},
                {"Total break time", formatDuration(s.totalBreakMinutes)},
            };
    }
}

}  // namespace

StatusScreen::StatusScreen(DISPLAY_CLASS &display) : display(display) {}

bool StatusScreen::importantChange(const SystemStatus &previous, const SystemStatus &next) {
    return previous.wifiConnected != next.wifiConnected || previous.ip != next.ip ||
           previous.timeSynced != next.timeSynced || previous.mqttConnected != next.mqttConnected;
}

void StatusScreen::draw() {
    const int16_t margin = 24;
    display.fillScreen(GxEPD_WHITE);

    // Header: "Status" on the left, page name and position on the right.
    drawText(display, "Status", margin, 56, &MAIN_FONT, GxEPD_BLACK);
    const String pageLabel = String(PAGE_TITLES[page]) + "  " + String(page + 1) + "/" + String(PAGE_COUNT);
    const Bounds pageBounds = getBounds(display, pageLabel.c_str(), &SUB_FONT);
    drawText(display, pageLabel.c_str(), display.width() - margin - pageBounds.w, 52, &SUB_FONT, GxEPD_BLACK);
    display.drawFastHLine(margin, 76, display.width() - 2 * margin, GxEPD_BLACK);

    // Rows: label left, value right-aligned.
    int16_t y = 128;
    for (const Row &row : rowsFor(page, status)) {
        drawText(display, row.label.c_str(), margin, y, &SUB_FONT, GxEPD_BLACK);
        const Bounds valueBounds = getBounds(display, row.value.c_str(), &SECONDARY_FONT);
        drawText(display, row.value.c_str(), display.width() - margin - valueBounds.w, y, &SECONDARY_FONT, GxEPD_BLACK);
        drawPattern(display, Pattern::Dots, margin, y + 14, display.width() - 2 * margin, 2);
        y += 54;
    }

    drawCenteredText(display, "Turn the knob for more pages - press to go back", display.width() / 2,
                     display.height() - 24, &SMALL_FONT, GxEPD_BLACK);
}

void StatusScreen::run(volatile const int *encoderCount) {
    status = collectSystemStatus();
    page = 0;
    draw();
    display.display(false);

    int lastEncoderCount = *encoderCount;
    unsigned long lastCollect = millis();
    unsigned long lastDraw = millis();

    while (true) {
        mqttLoop();

        if (Button::instance && Button::instance->checkAndClearButtonPress()) {
            return;
        }

        const int encoder = *encoderCount;
        if (encoder != lastEncoderCount) {
            page = encoder > lastEncoderCount ? (page + 1) % PAGE_COUNT : (page + PAGE_COUNT - 1) % PAGE_COUNT;
            lastEncoderCount = encoder;
            status = collectSystemStatus();
            lastCollect = lastDraw = millis();
            draw();
            display.display(true);
            continue;
        }

        if (millis() - lastCollect >= COLLECT_INTERVAL) {
            const SystemStatus fresh = collectSystemStatus();
            const bool redrawNow = importantChange(status, fresh) || millis() - lastDraw >= SLOW_REDRAW_INTERVAL;
            status = fresh;
            lastCollect = millis();
            if (redrawNow) {
                ESP_LOGD(TAG, "run: redrawing page %u", page);
                draw();
                display.display(true);
                lastDraw = millis();
            }
        }

        delay(10);
    }
}
