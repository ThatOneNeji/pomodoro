/**
 * @file button.h
 * @brief Interrupt-driven, debounced push-button handling for the encoder's optional switch.
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/**
 * @brief Debounced push-button wired to a GPIO interrupt.
 *
 * Only a single instance is supported at a time; the static ::instance pointer is what the
 * interrupt handler uses to reach the object, since ISRs cannot carry a `this` argument.
 */
class Button {
   private:
    int pin;                     ///< GPIO pin the button is wired to.
    static bool instanceExists;  ///< Guards against constructing more than one Button.

   public:
    volatile bool pressed = false;             ///< Set by the ISR when a debounced press is detected.
    volatile unsigned long lastPressTime = 0;  ///< millis() timestamp of the last accepted press, used for debouncing.

    static Button *instance;  ///< The single live Button instance, used by the static ISR to reach instance state.

    /**
     * @brief Configure @p pin as an input with an interrupt-driven press handler.
     * @param pin GPIO pin the button is wired to (pulled up internally, active low).
     */
    explicit Button(int pin);

    /// Detaches the interrupt and releases the singleton slot.
    ~Button();

    /**
     * @brief Interrupt service routine invoked on a falling edge of the button pin.
     *
     * Debounces presses in-place using ::lastPressTime and sets ::pressed. Placed in IRAM since
     * ISRs on the ESP32 must not be paged out of flash.
     */
    static void IRAM_ATTR buttonInterruptHandler();

    /**
     * @brief Consume a pending press, if any.
     * @return true if a press was pending (and has now been cleared), false otherwise.
     */
    bool checkAndClearButtonPress();
};
#endif