#pragma once

#include <Arduino.h>

/*   D5/A0/ADC0/PB5 RESET  -|1*  8|-  VCC  
 *   D3/A3/ADC1/PB3        -|2   7|-  SCL/SCK    D2/A1/PB2
 *   D4/A2/ADC2/PB4        -|3   6|-  MISO       D1/PB1
 *                  GND    -|4   5|-  MOSI SDA   D0/PB0
 */

#define PIN_D0      0
#define PIN_D1      1
#define PIN_D2      2
#define PIN_D3      3
#define PIN_D4      4
#define PIN_D5      5

#define PINS_Dx     { PIN_D0, \
                      PIN_D1, \
                      PIN_D2, \
                      PIN_D3, \
                      PIN_D4, \
                      PIN_D5 }

#define SETUP_LED   2
#define SETUP_PIN   PIN_D1

namespace app {

enum Result {
    RESULT_OK = 0,
    RESULT_FAILED,
    RESULT_NOENT
};

// -------------------------------------------------------

class PinBase {
public:
    PinBase(uint8_t pin): mPin(pin) {};
    ~PinBase() = default;
protected:
    uint8_t mPin;
};

} // namespace fm