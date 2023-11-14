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
#include "Checksum.h"

#include "Wire.h"

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define I2C_ADDR            0x0A

#define MAX_ACTIVE_TIME     30
#define MIN_SLEEP_TIME      60
#define MAX_SLEEP_TIME      3600
#define DEFAULT_SLEEP_TIME  900

#define TIME_INVALID        0

volatile bool isShutdownEventRcvd = false;
volatile uint16_t shutdownInterval = DEFAULT_SLEEP_TIME;

volatile bool isShutdownIntervalValid = false;
volatile bool isCurrentTimeValid = false;

volatile uint8_t checksumBits = 0;

app::PinOutput powerPin(PIN_D4);
app::PinVolts batteryPin(PIN_D3, 10.0, 36.0);

using namespace app;

void onEvent(int size) {

    (void)size;

    uint8_t maxAttempts = 20;

    while (maxAttempts != 0) {

        if ((Wire.available() == 0) ||
            (Wire.read() != I2C_ADDR)) {

            --maxAttempts;

            delay(50);
            continue;
        }

        switch(Wire.read()) {
            
            case 'S': {

                if (isShutdownEventRcvd == false) {

                    uint8_t checksum = 0;

                    shutdownInterval = DEFAULT_SLEEP_TIME;
                    isShutdownIntervalValid = false;

                    while (maxAttempts != 0) {
                        
                        if (Wire.available() >= 3) {

                            shutdownInterval  = ((uint16_t)Wire.read());
                            shutdownInterval |= ((uint16_t)Wire.read()) << 8;
                            checksum = (uint8_t)Wire.read();

                            break;
                        }
                        else {
                            --maxAttempts;
                            delay(30);
                        }
                    }

                    if (checksum == CRC('S', CRC16(shutdownInterval))) {

                        if (shutdownInterval == 0) {
                            shutdownInterval = DEFAULT_SLEEP_TIME;
                        }

                        isShutdownEventRcvd = true;
                        isShutdownIntervalValid = true;
                    }
                }

                break;
            }

            case 'T': { /* datetime */

                uint8_t checksum = 0;
                uint32_t currentTime = 0;

                isCurrentTimeValid = false;

                while (maxAttempts != 0) {
                        
                    if (Wire.available() >= 5) {

                        currentTime  = ((uint32_t)Wire.read());
                        currentTime |= ((uint32_t)Wire.read()) << 8;
                        currentTime |= ((uint32_t)Wire.read()) << 16;
                        currentTime |= ((uint32_t)Wire.read()) << 24;
                        
                        checksum = (uint8_t)Wire.read();

                        break;
                    }
                    else {
                        --maxAttempts;
                        delay(30);
                    }
                }

                if (checksum == CRC('T', CRC32(currentTime))) {

                    app::Watchdog& wd = app::Watchdog::getInstance();
                    wd.setCurrentTime(currentTime);

                    isCurrentTimeValid = true;
                }
            }
            break;

            default:
                break;
        }

        break;
    }
}

void onRequest() {

    Watchdog& wd = Watchdog::getInstance();

    secs_t currentTime = wd.datetime();

    uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 1000);
    int16_t calibration = (int16_t)(wd.getCalibration() / 10);

    Wire.write(LOBYTE(batteryVolts));
    Wire.write(HIBYTE(batteryVolts));

    Wire.write(LOBYTE(calibration));
    Wire.write(HIBYTE(calibration));

    Wire.write(NBYTE(0, currentTime));
    Wire.write(NBYTE(1, currentTime));
    Wire.write(NBYTE(2, currentTime));
    Wire.write(NBYTE(3, currentTime));

    Wire.write((uint8_t)checksumBits);

    Wire.write((uint8_t)CRC(CRC16(batteryVolts),
                            CRC16(calibration),
                            CRC32(currentTime),
                            CRC8(checksumBits)));
}

void setup() {

    Watchdog& wd = Watchdog::getInstance();
    wd.setCalibration(-50 * Watchdog::MSEC_US); // MS per second

    Wire.begin(I2C_ADDR);
    Wire.onReceive(onEvent);
    Wire.onRequest(onRequest);

    delay(1000);
}

void loop() {

    Watchdog& wd = Watchdog::getInstance();
    
    checksumBits = ((isCurrentTimeValid)      ? 0x01 : 0x00) |
                   ((isShutdownIntervalValid) ? 0x02 : 0x00);

    isShutdownEventRcvd = false;
    isShutdownIntervalValid = false;

    isCurrentTimeValid = false;

    powerPin.on();
    
    msec_t maxActiveTimeSecs = MAX_ACTIVE_TIME;
    while ((maxActiveTimeSecs-- != 0) && (!isShutdownEventRcvd)) {
        delay(SECOND);
    };   

    powerPin.off();

    if (isShutdownEventRcvd == true) {

        uint32_t sleepDuration = shutdownInterval;

        if (isCurrentTimeValid) {

            sleepDuration = RANGE(shutdownInterval, MIN_SLEEP_TIME, MAX_SLEEP_TIME);
        }

        wd.powerDown(sleepDuration, shutdownInterval);
    }
    else {

        wd.powerDown(DEFAULT_SLEEP_TIME);
    }
}

ISR ( WDT_vect ) {

    Watchdog& wd = Watchdog::getInstance();
    wd.onInterruptEvent();
}