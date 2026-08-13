/**
 * @file config.h
 * @brief Board wiring (e-paper SPI pins) and user-facing build-time options.
 */

/// @name E-paper display pin wiring
/// @{
#define EPD_SCK_PIN 18   ///< SPI clock pin.
#define EPD_MOSI_PIN 23  ///< SPI MOSI (data) pin.
#define EPD_CS_PIN 5     ///< SPI chip-select pin.
#define EPD_RST_PIN 16   ///< Display reset pin.
#define EPD_DC_PIN 17    ///< Data/command select pin.
#define EPD_BUSY_PIN 4   ///< Display busy-status input pin.
#define EPD_PWR_PIN 22   ///< Display power-enable output pin.
/// @}

/**
 * @brief Change this to 'true' to use the old style of displaying the remaining time
 *
 */
const static bool timerStyleOld = false;

// #define LANG_AF // Uncomment for Afrikaans text messages
// #define LANG_DE // Uncomment for German text messages
