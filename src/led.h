/**
 * @file led.h
 * @brief Status LED animations, driven by a dedicated FreeRTOS task pinned to core 0.
 *
 * The LED runs independently of the main loop (which drives the display on core 1) so its
 * fade animations stay smooth regardless of how long a display redraw takes.
 */
#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>

/// Selects which animation/behavior the LED task should run.
enum class LedMode {
    Off,                        ///< LED off.
    Splashscreen,               ///< Slow ambient fade cycle shown during the boot splash screen.
    QuickAcknowledgementFlash,  ///< Brief flash acknowledging a button press or encoder turn, then reverts to the
                                ///< previous mode.
    ConfirmationFlash,          ///< Slow pulsing flash used while waiting for the user to confirm an action.
    TimerPaused,                ///< Steady dim blue, shown while the timer is paused.
};

/// Initialize the LED strip and start the background animation task.
void setupLed();

/**
 * @brief Give the LED task a pointer to the shared encoder count, so it can flash on encoder input.
 * @param encoderCount Pointer to the same debounced encoder count the main loop reads.
 */
void ledSetupEncoder(volatile int *encoderCount);

/// Stop and delete the LED animation task.
void stopLed();

/// Create and start the LED animation task (see ::ledTaskHandle).
void startLedTask();

/**
 * @brief Switch the LED to a new mode/animation.
 * @param mode The mode to switch to.
 */
void setLedMode(LedMode mode);

/// Handle of the background FreeRTOS task that drives LED animations.
extern TaskHandle_t ledTaskHandle;

#endif