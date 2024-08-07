#pragma once

#include <Arduino.h>

#define GPIO_0  0
#define GPIO_1  1
#define GPIO_2  2
#define GPIO_3  3
#define GPIO_4  4
#define GPIO_5  5
#define GPIO_9  9
#define GPIO_10 10
#define GPIO_12 12
#define GPIO_13 13
#define GPIO_14 14
#define GPIO_15 15
#define GPIO_16 16

#if defined(BUILD_ESP8266)

#define PIN_D0      GPIO_16
#define PIN_D1      GPIO_5
#define PIN_D2      GPIO_4
#define PIN_D3      GPIO_0
#define PIN_D4      GPIO_2
#define PIN_D5      GPIO_14
#define PIN_D6      GPIO_12
#define PIN_D7      GPIO_13
#define PIN_D8      GPIO_15
#define PIN_RX      GPIO_3
#define PIN_TX      GPIO_1

#define PIN_LED     PIN_D4
#define PIN_SDA     PIN_D2
#define PIN_SCL     PIN_D1

#elif defined(ESP01)

/*   D5/A0/ADC0/PB5 RESET  -|1*  8|-  VCC
 *   D3/A3/ADC1/PB3        -|2   7|-  SCL/SCK    D2/A1/PB2
 *   D4/A2/ADC2/PB4        -|3   6|-  MISO       D1/PB1
 *                  GND    -|4   5|-  MOSI SDA   D0/PB0
 */

#define PIN_GPIO0   0
#define PIN_GPIO2   2

#define PIN_LED     PIN_GPIO2

#define PIN_SDA     PIN_GPIO0
#define PIN_SCL     PIN_GPIO2

#endif

namespace app {

class PinBase {
public:
    PinBase(uint8_t pin): mPin(pin) {};
    ~PinBase() = default;
protected:
    uint8_t mPin;
};

} // namespace app