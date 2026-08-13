/**
 * @file debug.h
 * @brief Compile-time switches for optional debug drawing and standalone hardware/UI test modes.
 *
 * All flags are commented out by default. Uncomment one at a time; the corresponding `#if`
 * block it guards (mostly in main.cpp) takes over `setup()` and loops forever, so these are
 * mutually exclusive, one-off test modes rather than something enabled in normal builds.
 */
#ifndef DEBUG_H

#define DEBUG_H

// Enables extra on-screen debug overlays (bounding boxes, crosshairs) across the UI.
// #define DEBUG

// Renders one icon at each supported scale side by side, to sanity-check scaled bitmaps.
// #define ICON_SCALING_TEST 1
// Renders every fill Pattern in a grid, to sanity-check pattern tiling.
// #define PATTERN_TEST 1
// Cycles through the break background images on the display, to sanity-check image assets.
// #define IMAGE_CYCLE_TEST 1
// Runs an interactive loop for exercising the Checkbox settings UI in isolation.
// #define CHECKBOX_TEST 1
// Prints every localized message and flags any that would overflow their allotted text area.
// #define STRINGS_TEST 1

#endif