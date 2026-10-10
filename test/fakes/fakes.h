/**
 * @file fakes.h
 * @brief Test-side controls for the host fakes: the in-memory settings store, and resetting
 * everything between tests. Fake time is fake::now, in Arduino.h.
 */
#pragma once

#include <Arduino.h>

#include <map>
#include <string>

namespace fake {
/// Stand-in for the settings in flash: every pref_* call reads and writes here.
inline std::map<std::string, unsigned long> prefs;
/// How many pref_put* calls were made, i.e. how many times flash would have been written.
inline int prefWrites = 0;

/// Clear the fake time and settings, for a fresh start in each test.
inline void reset() {
    now = 0;
    prefs.clear();
    prefWrites = 0;
}
}  // namespace fake
