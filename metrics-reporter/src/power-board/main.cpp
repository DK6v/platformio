#include <Arduino.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include "PinLed.h"
#include "PinOutput.h"
#include "PinCounter.h"
#include "PinVolts.h"
#include "PinI2C.h"
#include "Watchdog.h"
#include "Timer.h"
#include "Checksum.h"

#include "Wire.h"

#define START_BLINKS (0)

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define I2C_ADDR 0x0A

#define MAX_ACTIVE_TIME     60
#define MIN_SLEEP_TIME      30
#define MAX_SLEEP_TIME      (60 * 60)
#define DEFAULT_SLEEP_TIME  (15 * 60)

#define TIME_INVALID 0

using namespace app;

volatile uint32_t uptimeDurationMs = 0;

volatile bool isShutdownEventRcvd = false;
volatile uint16_t shutdownInterval = DEFAULT_SLEEP_TIME;

volatile bool isShutdownIntervalValid = false;
volatile bool isCurrentTimeValid = false;

volatile uint8_t checksumBits = 0;

app::PinOutput powerPin(PIN_D4);

void onEvent(int size)
{
    (void)size;

    uint8_t maxAttempts = 20;

    while (maxAttempts != 0)
    {
        if ((Wire.available() == 0) ||
            (Wire.read() != I2C_ADDR))
        {
            --maxAttempts;

            delay(50);
            continue;
        }

        switch (Wire.read())
        {

        case 'S':
        {
            if (isShutdownEventRcvd == false)
            {
                uint8_t checksum = 0;

                shutdownInterval = DEFAULT_SLEEP_TIME;
                isShutdownIntervalValid = false;

                while (maxAttempts != 0)
                {
                    if (Wire.available() >= 3)
                    {
                        shutdownInterval = ((uint16_t)Wire.read());
                        shutdownInterval |= ((uint16_t)Wire.read()) << 8;
                        checksum = (uint8_t)Wire.read();

                        break;
                    }
                    else
                    {
                        --maxAttempts;
                        delay(30);
                    }
                }

                if (checksum == CRC('S', CRC16(shutdownInterval)))
                {
                    isShutdownEventRcvd = true;
                    isShutdownIntervalValid = true;
                }
                else {
                    shutdownInterval = DEFAULT_SLEEP_TIME;
                }
            }

            break;
        }

        case 'T': // date and time
        {
            uint8_t checksum = 0;
            uint32_t currentTime = 0;

            isCurrentTimeValid = false;

            while (maxAttempts != 0)
            {
                if (Wire.available() >= 5)
                {
                    currentTime = ((uint32_t)Wire.read());
                    currentTime |= ((uint32_t)Wire.read()) << 8;
                    currentTime |= ((uint32_t)Wire.read()) << 16;
                    currentTime |= ((uint32_t)Wire.read()) << 24;

                    checksum = (uint8_t)Wire.read();

                    break;
                }
                else
                {
                    --maxAttempts;
                    delay(30);
                }
            }

            if (checksum == CRC('T', CRC32(currentTime)))
            {
                app::Watchdog &wd = app::Watchdog::getInstance();
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

void onRequest()
{
    Watchdog &wd = Watchdog::getInstance();
    secs_t currentTime = wd.datetime();

    app::PinVolts batteryPin(PIN_D3, 9.9, 43.0);
    uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 1000);
    int16_t calibration = (int16_t)(wd.getCalibration() / 10);

    uint32_t tempVar = 0x01;

    Wire.write(LOBYTE(batteryVolts));
    Wire.write(HIBYTE(batteryVolts));

    Wire.write(LOBYTE(calibration));
    Wire.write(HIBYTE(calibration));

    Wire.write(NBYTE(0, currentTime));
    Wire.write(NBYTE(1, currentTime));
    Wire.write(NBYTE(2, currentTime));
    Wire.write(NBYTE(3, currentTime));

    Wire.write(NBYTE(0, uptimeDurationMs));
    Wire.write(NBYTE(1, uptimeDurationMs));
    Wire.write(NBYTE(2, uptimeDurationMs));
    Wire.write(NBYTE(3, uptimeDurationMs));

    Wire.write((uint8_t)checksumBits);

    Wire.write((uint8_t)CRC(CRC16(batteryVolts),
                            CRC16(calibration),
                            CRC32(currentTime),
                            CRC32(uptimeDurationMs),
                            CRC8(checksumBits)));  
}

void setup()
{
    powerPin.off();
    delay(500);

    Watchdog &wd = Watchdog::getInstance();
    wd.setCalibration(-50 * Watchdog::MSEC_US); // MS per second
}

void loop()
{
    unsigned long startTimeMs = millis();
    Watchdog &wd = Watchdog::getInstance();

    checksumBits = ((isCurrentTimeValid) ? 0x01 : 0x00) |
                   ((isShutdownIntervalValid) ? 0x02 : 0x00);

    isShutdownEventRcvd = false;
    isShutdownIntervalValid = false;

    isCurrentTimeValid = false;

    Wire.begin(I2C_ADDR);
    Wire.onReceive(onEvent);
    Wire.onRequest(onRequest);

    powerPin.on();

    uint16_t maxWaitAttemps = MAX_ACTIVE_TIME * 10;
    while ((maxWaitAttemps-- != 0) && (!isShutdownEventRcvd))
    {
        delay(100);
    };

    pinMode(PB0, INPUT);
    pinMode(PB1, INPUT);
    pinMode(PB2, INPUT);
    pinMode(PB3, INPUT);

    powerPin.off();
    uptimeDurationMs = static_cast<uint32_t>(millis() - startTimeMs);

    if ((isShutdownEventRcvd == true) &&
        (isShutdownIntervalValid == true))
    {
        uint32_t sleepDuration = RANGE(shutdownInterval, MIN_SLEEP_TIME, MAX_SLEEP_TIME);
        wd.powerDown(sleepDuration, shutdownInterval);
    }
    else
    {
        wd.powerDown(DEFAULT_SLEEP_TIME);
    }
}

ISR(WDT_vect, ISR_BLOCK)
{
}

ISR(SIG_PIN_CHANGE, ISR_BLOCK)
{
    Watchdog &wd = Watchdog::getInstance();
    wd.onInterruptEvent();
}