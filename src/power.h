/**
 * @file power.h
 * @brief Switching the device off (Settings → Shut down), and back on with the knob.
 */
#ifndef POWER_H
#define POWER_H

#include "config.h"

/**
 * @brief Switch the device off: tell web clients, clear the screen and the LED, then deep sleep.
 *
 * The ESP32 can't cut its own power, so "off" is deep sleep with everything else powered down
 * or cleared. Pressing the knob button (::ENCODER_SW) wakes it, which restarts the firmware from
 * the beginning, as if it had just been plugged in. Doesn't return.
 *
 * Order matters: the WebSocket `shutdown` message is queued first, so it can go out while the
 * panel clears (a few seconds); then the connections are closed cleanly.
 * @param display The e-paper display, cleared to white so it doesn't keep the last screen.
 */
[[noreturn]] void shutDown(DISPLAY_CLASS &display);

#endif  // POWER_H
