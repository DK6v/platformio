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

#define NBYTE(n, x)     ((uint8_t)(((x) >> (8 * ((n) - 1))) & 0xFF))
#define LOBYTE(x)       NBYTE(1, x)
#define HIBYTE(x)       NBYTE(2, x)

#define CRC16(x)        (NBYTE(1, x) ^ NBYTE(2, x))
#define CRC32(x)        (NBYTE(1, x) ^ NBYTE(2, x) ^ NBYTE(3, x) ^ NBYTE(4, x))

#define I2C_ADDR        0x0A

#define MAX_ACTIVE_TIME     60
#define MIN_SLEEP_TIME      60
#define MAX_SLEEP_TIME      3600
#define DEFAULT_SLEEP_TIME  180

#define TIME_INVALID        0

volatile unsigned long startTicks = 0;
volatile unsigned long currentTicks = 0;

volatile bool isShutdownEventRcvd = false;
volatile uint16_t shutdownInterval = DEFAULT_SLEEP_TIME;

volatile uint32_t currentTime = 0;
volatile uint32_t expectedTime = 0;

volatile bool isShutdownIntervalValid = false;
volatile bool isCurrentTimeValid = false;

volatile uint8_t checksumBits = 0;

app::PinOutput powerPin(PIN_D4);
app::PinVolts batteryPin(PIN_D3, 10.0, 36.0);

void onEvent(int size) {

    (void)size;

    while(Wire.available()) {

        uint8_t checksum = 0;

        if (Wire.read() != I2C_ADDR) {
            continue;
        }

        switch(Wire.read()) {
            
            case 'S': {

                if (isShutdownEventRcvd == false) {

                    shutdownInterval = 0;
                    isShutdownIntervalValid = false;

                    unsigned int wait = 30;
                    while((wait--) != 0) {

                        if (Wire.available()) {
                            shutdownInterval  = ((uint16_t)Wire.read());
                            shutdownInterval |= ((uint16_t)Wire.read()) << 8;
                            checksum = (uint8_t)Wire.read();
                            break;
                        }

                        delay(100);
                    }

                    if ((shutdownInterval != 0) && 
                        (checksum == ('S' ^ CRC16(shutdownInterval)))) {

                        isShutdownIntervalValid = true;
                    }
                    else {
                        shutdownInterval = DEFAULT_SLEEP_TIME;
                    }
                
                    isShutdownEventRcvd = true;
                }

                break;
            }

            case 'T': { /* datetime */

                currentTime = 0;
                currentTicks = 0;
                isCurrentTimeValid = false;

                unsigned int wait = 30;
                while((wait--) != 0) {

                    if (Wire.available()) {
                        currentTime  = ((uint32_t)Wire.read());
                        currentTime |= ((uint32_t)Wire.read()) << 8;
                        currentTime |= ((uint32_t)Wire.read()) << 16;
                        currentTime |= ((uint32_t)Wire.read()) << 24;
                        checksum = (uint8_t)Wire.read();
                        break;
                    }
                    delay(100);
                }

                if ((currentTime != 0) &&
                    (checksum == ('T' ^ CRC32(currentTime)))) {

                    app::Watchdog& wd = app::Watchdog::getInstance();
                    // app::secs_t duration = (app::secs_t)((millis() - startTicks) / app::SECOND);

                    if (expectedTime != TIME_INVALID) {

                        // expectedTime += RANGE(duration, 0, MAX_ACTIVE_TIME);
                        wd.calibrate(shutdownInterval, currentTime - expectedTime);
                    }

                    currentTicks = millis();
                    isCurrentTimeValid = true;
                }
                else {
                    currentTime = TIME_INVALID;
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

    app::Watchdog& wd = app::Watchdog::getInstance();

    uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 1000);
    int16_t calibration = (int16_t)(wd.getCalibration() / 10);

    Wire.write(LOBYTE(batteryVolts));
    Wire.write(HIBYTE(batteryVolts));
    Wire.write(LOBYTE(calibration));
    Wire.write(HIBYTE(calibration));
    Wire.write((uint8_t)checksumBits);

    Wire.write(CRC16(batteryVolts ^ calibration ^ (uint16_t)checksumBits));
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
    
    checksumBits = ((isCurrentTimeValid)      ? 0x01 : 0x00) |
                   ((isShutdownIntervalValid) ? 0x02 : 0x00);

    isShutdownEventRcvd = false;
    isShutdownIntervalValid = false;

    startTicks = millis();
    isCurrentTimeValid = false;

    powerPin.on();
    
    app::msec_t activeInterval = MAX_ACTIVE_TIME;
    while ((activeInterval-- != 0) && (!isShutdownEventRcvd)) {
        delay(app::SECOND);
    };   

    if (isShutdownEventRcvd == true) {

        app::secs_t duration = (app::secs_t)((millis() - currentTicks) / app::SECOND);
        duration = RANGE(duration, 0, MAX_ACTIVE_TIME);

        uint32_t sleepDuration = RANGE(shutdownInterval - duration, MIN_SLEEP_TIME, MAX_SLEEP_TIME);
        expectedTime = (isCurrentTimeValid) ? (currentTime + shutdownInterval) : TIME_INVALID;

        powerPin.off();
        wd.sleep(sleepDuration);
    }
    else {
        powerPin.off();
        wd.sleep(shutdownInterval);
    }
}

ISR ( WDT_vect ) {

    app::Watchdog& wd = app::Watchdog::getInstance();
    wd.onInterruptEvent();
}