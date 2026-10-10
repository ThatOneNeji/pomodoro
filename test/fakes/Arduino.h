/**
 * @file Arduino.h
 * @brief Host-test stand-in for the Arduino core: just what the firmware sources under test use.
 *
 * Only on the include path for `pio test -e native` (see platformio.ini), never for the ESP32
 * build. Time is fake: millis() returns fake::now, which tests move forward by hand.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "WString.h"

#define IRAM_ATTR
#define PROGMEM

using std::max;
using std::min;

typedef void *TaskHandle_t;

namespace fake {
/// What millis() returns. Tests set or advance it; nothing advances it on its own.
inline unsigned long now = 0;
}  // namespace fake

inline unsigned long millis() { return fake::now; }

/// Deterministic: always the lowest value, so message picks are repeatable.
inline long random(long min, long) { return min; }
inline long random(long) { return 0; }
