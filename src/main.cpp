/**
 * @file main.cpp
 * @brief Arduino entry point: hardware bring-up (setup()) and the main event loop (loop()).
 */
#include <Arduino.h>
#include "esp_log.h"

#include "config.h"
#include <ESP32Encoder.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include "GxEPD2_display_selection_new_style.h"
#include "timer.h"
#include "led.h"
#include "debug.h"
#include "images.h"
#include "checkbox.h"
#include "splashscreen.h"
#include "button.h"
#include "icon_provider.h"
#include "anniversary.h"
#include "preferences_manager.h"
#include "web_server.h"

#if STRINGS_TEST
#include <sstream>
#endif

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "MAIN";

ESP32Encoder encoder;                          ///< Rotary encoder hardware driver.
volatile unsigned long lastEncoderUpdate = 0;  ///< millis() timestamp checkPosition() last accepted a count update.
const unsigned long encoderDebounceTime = 10;  ///< Minimum ms between accepted encoder count updates.
volatile int debouncedCount = 0;  ///< Debounced encoder count, shared with Timer::loop()/SplashScreen/Menu.
volatile int lastCount = 0;       ///< debouncedCount as of the end of the previous loop() iteration.

// Declared but unused: no code currently reads these.
volatile int tempCount = 0;                      ///< Unused.
volatile unsigned long tempCountTime = 0;        ///< Unused.
volatile bool positionStable = true;             ///< Unused.
volatile bool buttonPressed = false;             ///< Unused.
volatile unsigned long lastButtonPressTime = 0;  ///< Unused.

#if CHECKBOX_TEST
Checkbox checkbox(&icon_lpehacker, "A test", "test");
Checkbox checkbox2(&icon_lpetantrum, "Another test", "test2");
#endif

/**
 * @brief Rotary encoder ISR: debounces the raw quadrature count into ::debouncedCount.
 *
 * Ignores changes immediately after a button press (see ENCODER_STABILITY_DELAY) since the
 * button press can jostle the encoder, and only accepts an updated count once it settles for
 * at least `encoderDebounceTime`. Placed in IRAM since ISRs on the ESP32 must not be paged out
 * of flash.
 * @param arg Unused (required by the ESP32Encoder callback signature).
 */
void IRAM_ATTR checkPosition(void *arg) {
    unsigned long currentTime = millis();
    int currentCount = encoder.getCount();

    // If there was a recent button press, ignore encoder changes
    if (currentTime - Button::instance->lastPressTime < ENCODER_STABILITY_DELAY) {
        return;
    }

    if (currentCount % 2 == 0 && currentCount != debouncedCount &&
        (currentTime - lastEncoderUpdate >= encoderDebounceTime)) {
        debouncedCount = (currentCount % 2 == 0 ? currentCount : currentCount - 1) / 2;
        lastEncoderUpdate = currentTime;
    }
}

/// Configure the rotary encoder pins and attach checkPosition() as its half-quadrature callback.
void setupEncoder() {
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);

    encoder = ESP32Encoder(true, checkPosition);
    ESP32Encoder::useInternalWeakPullResistors = puType::none;

    encoder.attachHalfQuad(ENCODER_DT, ENCODER_CLK);
    encoder.setFilter(1023);

    encoder.clearCount();
    debouncedCount = 0;
    lastCount = 0;
}

/**
 * @brief Forces NVS to be initialized before any other global object's constructor runs.
 *
 * ::timer's constructor resolves several menu-item message strings, which reads a preference
 * (see MessageCache::isLpeModeEnabled()) — and global objects are constructed before setup()
 * ever runs, so without this, those reads would hit NVS before initPreferences() (called from
 * setup()) has had a chance to run, failing with NOT_INITIALIZED. C++ guarantees globals in the
 * same file initialize in declaration order, so declaring this immediately before ::timer below
 * is what makes the ordering safe.
 */
struct PreferencesInitializer {
    PreferencesInitializer() { initPreferences(); }
};
static PreferencesInitializer preferencesInitializer;

/// The global Timer instance driving the whole UI, constructed against the display selected in
/// GxEPD2_display_selection_new_style.h.
Timer timer(display);

/**
 * @brief Arduino setup(): bring up hardware, run the splash screen, and register the timer presets.
 *
 * Order matters here: button/LED/encoder/preferences are initialized before the display, and
 * the splash screen's blocking loop() (see SplashScreen::loop()) runs to completion — either
 * starting the timer or falling into the settings screen and restarting the device — before
 * this function returns and the main loop() begins driving Timer::loop().
 */
void setup() {
    Serial.begin(115200);
    pinMode(EPD_PWR_PIN, OUTPUT);
    digitalWrite(EPD_PWR_PIN, HIGH);

    Button::instance = new Button(ENCODER_SW);
    ledSetupEncoder(&debouncedCount);

    setupLed();

    // Initialize the encoder
    setupEncoder();

    // Initialize preferences once
    initPreferences();

    // Start connecting to WiFi in the background; the webserver comes online later, once (and
    // only if) a connection succeeds. Never blocks startup.
    setupWebServer();

    // Initialize the display
    display.init(115200, true, 2, false);
    display.setRotation(0);

    // Load LPE mode setting from checkbox
    IconProvider::getInstance()->setLpeMode(pref_getCheckbox("lpe", true));

#ifdef DEBUG
#if ICON_SCALING_TEST
    const uint16_t padding = 20;

    const auto sizes = {48, 64, 128, 192};
    uint16_t xOffset = padding;
    for (const auto size : sizes) {
        const auto icon = icon_lpesip.scaled(size);
        const uint16_t yOffset = padding + 192 - size;
        // draw each icon size side by side left to right
        display.drawBitmap(xOffset, padding, icon.data, size, size, GxEPD_BLACK);
        drawCenteredText(display, String(size).c_str(), xOffset + size / 2, size + 2 * padding, &SUB_FONT, GxEPD_BLACK);

        drawCenteredText(display, "#autoscaling", display.width() / 2, display.height() / 2 + 48, &MAIN_FONT,
                         GxEPD_BLACK);

        xOffset += size + padding;
    }

    display.display();

    while (true)
        ;
#endif

#if PATTERN_TEST

    // draw each pattern in a rounded area on a grid within display bounds
    const uint16_t padding = 10;
    const uint16_t w = display.width() / patterns.size() - 2 * padding;
    for (uint16_t i = 0; i < patterns.size(); i++) {
        drawPatternInRoundedArea(display, i * display.width() / patterns.size() + padding, padding, w,
                                 display.height() - 2 * padding, 10, Pattern(i));

        char buffer[2];
        sprintf(buffer, "%d", i);
        drawCenteredText(display, buffer, i * display.width() / patterns.size() + display.width() / patterns.size() / 2,
                         display.height() - padding - 10, &SUB_FONT, GxEPD_BLACK);
    }

    display.display();

    while (true)
        ;

#endif

#if IMAGE_CYCLE_TEST
    std::vector<const unsigned char *> images = {
        // image_bg_cat,
        image_bg_stonks,
        image_bg_pablo,
        image_bg_what_a_week,
    };

    display.fillScreen(GxEPD_WHITE);
    display.display();

    for (const auto image : images) {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, image, display.width(), display.height(), GxEPD_BLACK);
        display.display(false);
        display.drawBitmap(0, 0, image, display.width(), display.height(), GxEPD_BLACK);
        display.display(true);
        display.drawBitmap(0, 0, image, display.width(), display.height(), GxEPD_BLACK);
        display.display(true);

        delay(5000);
    }
#endif

#if CHECKBOX_TEST
    display.fillScreen(GxEPD_WHITE);

    Checkbox *selected = &checkbox;

    checkbox.load();
    checkbox2.load();

    checkbox.draw(display, 0, 0, display.width(), 96, selected == &checkbox);
    checkbox2.draw(display, 0, 96 + 8, display.width(), 96, selected == &checkbox2);
    display.display();
    while (true) {
        if (!digitalRead(ENCODER_SW)) {
            display.fillScreen(GxEPD_WHITE);

            selected->toggle();
            selected->save();

            checkbox.draw(display, 0, 0, display.width(), 96, selected == &checkbox);
            checkbox2.draw(display, 0, 96 + 8, display.width(), 96, selected == &checkbox2);
            display.display();
            delay(500);
        }

        if (debouncedCount != lastCount) {
            display.fillScreen(GxEPD_WHITE);

            selected->draw(display, 0, selected == &checkbox ? 0 : 96 + 8, display.width(), 96, false);
            selected = selected == &checkbox ? &checkbox2 : &checkbox;
            selected->draw(display, 0, selected == &checkbox ? 0 : 96 + 8, display.width(), 96, true);

            display.display();
        }

        lastCount = debouncedCount;
    }
#endif

#endif

#if STRINGS_TEST

    // Space for message: 221,330 until 649,409
    const uint16_t messageMinX = 221;
    const uint16_t messageMaxX = 649;
    const uint16_t messageMinY = 330;
    const uint16_t messageMaxY = 409;
    const uint16_t messageW = messageMaxX - messageMinX;
    const uint16_t messageH = messageMaxY - messageMinY;

    ESP_LOGI(TAG, "--- Messages ---");

    for (auto msg : messageCache.getMessages()) {
        // Split message by lines and print individually
        std::string messageStr(msg);
        std::istringstream iss(messageStr);
        std::string line;
        int lineIndex = 0;
        while (std::getline(iss, line, '\n')) {
            if (lineIndex >= 3) {
                ESP_LOGW(TAG, "Too many lines in message \"%s\"", msg);
                break;
            }

            int yPos = messageMinY + lineIndex * 18 + 18;  // + 18 because of the first line
            Bounds b = drawText(display, line.c_str(), messageMinX, yPos, &SMALL_FONT, GxEPD_BLACK);
            ++lineIndex;

            if (b.w > messageW || b.y + b.h > messageMaxY) {
                auto wOver = b.w - messageW;
                auto hOver = b.y + b.h - messageMaxY;

                ESP_LOGW(TAG, "Line too long: \"%s\" {x: %d, y: %d, w: %d (+%d), h: %d (+%d)}", line.c_str(), b.x, b.y,
                         b.w, wOver > 0 ? wOver : 0, b.h, hOver > 0 ? hOver : 0);
            } else {
                ESP_LOGI(TAG, "\"%s\" {x: %d, y: %d, w: %d, h: %d}", line.c_str(), b.x, b.y, b.w, b.h);
            }
        }
    }
#endif

#if ANNIVERSARY_MODE
    if (!pref_getCheckbox("anniversary", false)) {
        Anniversary anniversary(display);

        anniversary.loop();

        pref_putCheckbox("anniversary", true);
    }
#endif

    setLedMode(LedMode::Splashscreen);

    SplashScreen splashScreen(display, timer);
    splashScreen.draw();
    splashScreen.loop(&debouncedCount);
    setLedMode(LedMode::Off);

    auto iconProvider = IconProvider::getInstance();

    for (const auto &preset : PRESETS) {
        timer.addPreset(iconProvider->getPresetIcon(preset.name), iconProvider->getTimerRunningBackgroundImage(),
                        preset.name, preset.duration, preset.pauseDuration, preset.longPauseDuration,
                        preset.longPauseAfter);
    }

    // enterPresetSelection() (rather than selectPreset(1) alone) also resets the idle-blank
    // timer, which otherwise would have been ticking since Timer's construction near boot —
    // through however long setup() and the splash screen took — causing an immediate blank
    // on the very first tick of real interaction.
    timer.enterPresetSelection();
}

/// Arduino loop(): drive the timer state machine each iteration with the current encoder count.
void loop() {
    timer.loop(&debouncedCount);
    lastCount = debouncedCount;

    webServerLoop();
}
