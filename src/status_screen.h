/**
 * @file status_screen.h
 * @brief The Status pages (from the preset selection screen): network, time/MQTT, device, firmware and
 * storage/statistics.
 */
#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include <Arduino.h>
#include "config.h"
#include "system_status.h"

/**
 * @brief Read-only device status, opened from the preset selection screen's top row.
 *
 * run() blocks until dismissed: turning the knob pages through the groups, and
 * a press returns. The content refreshes once a second, but the e-paper is only redrawn when
 * something important changes (WiFi connects, the clock syncs, MQTT connects), and otherwise
 * every ::SLOW_REDRAW_INTERVAL for slowly drifting values like uptime and free memory.
 */
class StatusScreen {
   private:
    DISPLAY_CLASS &display;
    SystemStatus status{};  ///< The snapshot currently drawn.
    unsigned int page = 0;  ///< Index into the page list in status_screen.cpp.

    static const unsigned long COLLECT_INTERVAL = 1000;  ///< How often to re-collect the status, in ms.
    static const unsigned long SLOW_REDRAW_INTERVAL =
        30000;  ///< Redraw interval for values that only drift (uptime, memory, signal), in ms.

    /// Draw the current page into the display buffer (the caller pushes it to the panel).
    void draw();

    /// @return Whether @p next differs from @p previous in a way that's worth an immediate redraw.
    static bool importantChange(const SystemStatus &previous, const SystemStatus &next);

   public:
    /// @param display Shared e-paper display reference.
    explicit StatusScreen(DISPLAY_CLASS &display);

    /**
     * @brief Show the Status pages until the button is pressed.
     *
     * Also keeps MQTT serviced while open (this screen blocks loop() and so mqttLoop()), so the
     * MQTT row shows the real connection state.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     */
    void run(volatile const int *encoderCount);
};

#endif  // STATUS_SCREEN_H
