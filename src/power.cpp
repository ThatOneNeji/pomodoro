/// @file power.cpp
#include "power.h"
#include "gfx_utils.h"
#include "led.h"
#include "web_server.h"
#include "esp_log.h"
#include <driver/rtc_io.h>
#include <esp_sleep.h>

/// Log tag for this file, used by ESP_LOGx() calls.
[[maybe_unused]] static const char *TAG = "POWER";

void shutDown(DISPLAY_CLASS &display) {
    ESP_LOGW(TAG, "shutDown: switching off; pressing the knob switches the device back on");

    webServerAnnounceShutdown();
    // MQTT (roadmap, not built yet): publish "offline" to the availability topic here. The broker
    // only sends the last will for connections that drop unexpectedly, not for a clean shutdown.

    stopLed();
    clearPanel(display);  // also gives the network a few seconds to send the shutdown message
    digitalWrite(EPD_PWR_PIN, LOW);

    webServerDisconnectClients();
    delay(200);  // let the close frames go out

    // The button pulls the pin low while pressed. Sleeping while it's still held would wake the
    // device straight away, so wait for the press that chose "Shut down" to end.
    while (digitalRead(ENCODER_SW) == LOW) {
        delay(10);
    }
    delay(50);  // debounce the release

    rtc_gpio_pullup_en((gpio_num_t) ENCODER_SW);
    rtc_gpio_pulldown_dis((gpio_num_t) ENCODER_SW);
    esp_sleep_enable_ext0_wakeup((gpio_num_t) ENCODER_SW, 0);
    esp_deep_sleep_start();
}
