#include <Arduino.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "PinLed.h"
#include "PinOutput.h"
#include "PinVolts.h"
#include "PinI2C.h"
#include "Watchdog.h"
#include "Timer.h"

#include "Wire.h"

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define LOW_BYTE(x)     (uint8_t)((x) & 0xFF)
#define HIGH_BYTE(x)    (uint8_t)(((x) >> 8) & 0xFF)

#define I2C_ADDR        0x0A

#define MAX_ACTIVE_TIME 180
#define MIN_SLEEP_TIME  60
#define MAX_SLEEP_TIME  3600

extern volatile bool interruptReceived;

volatile bool shutdownEvent = false;
volatile uint16_t shutdownInterval = 0;

volatile uint32_t currentTime = 0;
volatile uint32_t expectedTime = 0;

app::PinOutput powerPin(PIN_D4);
app::PinVolts batteryPin(PIN_D3, 10.0, 36.0);

void onEvent(int size) {

    (void)size;

    while(Wire.available()) {

        if (Wire.read() != I2C_ADDR) {
            continue;
        }

        char c = Wire.read();
        switch(c) {
            
            case 'S': { /* shutdown */
                shutdownEvent = true;
                shutdownInterval = 0;

                unsigned int wait = 30;
                while((wait--) != 0) {

                    if (Wire.available() >= 2) {
                        shutdownInterval = ((uint16_t)Wire.read());
                        shutdownInterval += ((uint16_t)Wire.read()) << 8;
                        break;
                    }
                    delay(100);
                }
                break;
            }

            case 'T': { /* datetime */
                currentTime = 0;

                unsigned int wait = 30;
                while((wait--) != 0) {
                    if (Wire.available() >= 4) {
                        currentTime = ((uint32_t)Wire.read());
                        currentTime += ((uint32_t)Wire.read()) << 8;
                        currentTime += ((uint32_t)Wire.read()) << 16;
                        currentTime += ((uint32_t)Wire.read()) << 24;
                        break;
                    }
                    delay(100);
                }
            }
            break;

            default:
                (void)Wire.read(); // skip
                break;
        }
    }
}

void onRequest() {

    uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 100);

    Wire.write(LOW_BYTE(batteryVolts));
    Wire.write(HIGH_BYTE(batteryVolts));
}

void setup() {

    app::Watchdog& wd = app::Watchdog::getInstance();
    wd.resetCalibration();
    wd.setCalibration(-100000 /* usec */);

    Wire.begin(I2C_ADDR);
    Wire.onReceive(onEvent);
    Wire.onRequest(onRequest);

    delay(2000);
}

void loop() {

    app::Watchdog& wd = app::Watchdog::getInstance();
    app::msec_t startTime = millis();

    shutdownEvent = false;
 
    powerPin.on();
    
    app::secs_t activeInterval = MAX_ACTIVE_TIME;
    while (((activeInterval--) > 0) && (!shutdownEvent)) {
        delay(app::SECOND);
    };   

    if (shutdownEvent == true) {

        // shutdownInterval = 100;

        uint16_t duration = (uint16_t)((millis() - startTime) / app::SECOND);

        if ((currentTime != 0) && (expectedTime != 0)) {

            wd.calibrate(shutdownInterval, currentTime - expectedTime);
        }

        expectedTime = currentTime + shutdownInterval;
        currentTime = 0;

        if (shutdownInterval > duration) {

            shutdownInterval -= duration;
            shutdownInterval = RANGE(shutdownInterval, MIN_SLEEP_TIME, MAX_SLEEP_TIME);
        }

        powerPin.off();
        wd.sleep(shutdownInterval);
    }
    else {

        powerPin.off();
        wd.sleep(MIN_SLEEP_TIME);
    }
}

ISR ( WDT_vect ) {

    app::Watchdog& wd = app::Watchdog::getInstance();
    wd.onInterruptEvent();
}