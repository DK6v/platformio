#include <Arduino.h>

#include "Byte.h"
#include <RBufferHelper.h>
#include <WBufferHelper.h>
#include "PinCounter.h"
#include "PinOutput.h"
#include "PinVolts.h"
#include "Watchdog.h"

#include "Wire.h"

#define I2C_ADDR            0x0A

#define TIME_INVALID        0
#define MAX_ACTIVE_TIME     60
#define MIN_SLEEP_TIME      30
#define MAX_SLEEP_TIME      3600
#define DEFAULT_SLEEP_TIME  300

using namespace app;

volatile uint16_t g_restartCounter = 0;
volatile uint16_t g_shutdownInterval = DEFAULT_SLEEP_TIME;
volatile uint16_t g_shutdownTimeRound = 0;
volatile uint32_t g_uptimeDurationMs = 0;
volatile uint8_t g_connectionStatus = 0;
volatile uint8_t g_checksumBits = 0;

volatile bool g_isShutdownEventRcvd = false;
volatile bool g_isShutdownIntervalValid = false;
volatile bool g_isCurrentTimeValid = false;
volatile bool g_isConnectionStatusValid = false;

app::PinOutput powerPin(PIN_D4);

void onEvent(int size)
{
    (void)size;
    uint8_t maxAttempts = 20;

    auto rbuf = RBufferHelper<TwoWire*>(+[](TwoWire *ptr) -> char {
        return ptr->read();
    }, &Wire);

    while (maxAttempts != 0)
    {
        if ((Wire.available() == 0) || (Wire.read() != I2C_ADDR))
        {
            --maxAttempts;

            delay(50);
            continue;
        }

        switch (Wire.read())
        {
            case 'S':
            {
                if (g_isShutdownEventRcvd == false)
                {
                    uint8_t checksum = 0;

                    g_isShutdownEventRcvd = true;
                    g_isShutdownIntervalValid = false;

                    g_shutdownInterval = DEFAULT_SLEEP_TIME;
                    g_shutdownTimeRound = 0;

                    if (rbuf.getBit())
                    {
                        g_shutdownInterval = rbuf.getBits(15);
                    }

                    if (rbuf.getBit())
                    {
                        g_shutdownTimeRound = rbuf.getBits(15);
                    }

                    checksum = rbuf.getByte();

                    if (checksum == BYTE_XOR('S', BYTE16(g_shutdownInterval),
                                                  BYTE16(g_shutdownTimeRound)))
                    {
                        g_isShutdownIntervalValid = true;
                    }
                    else
                    {
                        g_shutdownInterval = DEFAULT_SLEEP_TIME;
                        g_shutdownTimeRound = 0;
                    }
                }

                break;
            }

            case 'T': // date and time
            {
                uint8_t checksum = 0;
                uint32_t currentTime = 0;

                g_isCurrentTimeValid = false;

                while (maxAttempts != 0)
                {
                    if (Wire.available() >= 5)
                    {
                        currentTime = rbuf.getBytes(4);
                        checksum = rbuf.getByte();
                        break;
                    }
                    else
                    {
                        --maxAttempts;
                        delay(30);
                    }
                }

                if (checksum == BYTE_XOR('T', BYTE32(currentTime)))
                {
                    app::Watchdog &wd = app::Watchdog::getInstance();
                    wd.setCurrentTime(currentTime);

                    g_isCurrentTimeValid = true;
                }
            }
            break;

            case 'R':
            {
                uint8_t checksum = 0;

                g_connectionStatus = 0;
                g_isConnectionStatusValid = false;

                while (maxAttempts != 0)
                {
                    if (Wire.available() >= 2)
                    {
                        g_connectionStatus = rbuf.getByte();
                        checksum = rbuf.getByte();
                        break;
                    }
                    else
                    {
                        --maxAttempts;
                        delay(30);
                    }
                }

                if (checksum == BYTE_XOR('R', BYTE8(g_connectionStatus)))
                {
                    g_isConnectionStatusValid = true;
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
    secs_t currentTime = wd.getCurrentTime();

    app::PinVolts batteryPin(PIN_D3, 9.9, 43.0);
    uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 1000);
    int16_t calibration = (int16_t)(wd.getCalibration() / 10);

    auto wbuf = WBufferHelper<TwoWire*>(+[](char value, TwoWire *ptr) -> void {
        ptr->write(value);
    }, &Wire);

    wbuf.setBytes(2, g_restartCounter);
    wbuf.setBytes(2, batteryVolts);
    wbuf.setBytes(2, calibration);
    wbuf.setBytes(4, g_uptimeDurationMs);
    wbuf.setBytes(1, g_connectionStatus);
    wbuf.setBytes(1, g_checksumBits);

    wbuf.setByte(BYTE_XOR(BYTE16(g_restartCounter),
                          BYTE16(batteryVolts),
                          BYTE16(calibration),
                          BYTE32(g_uptimeDurationMs),
                          BYTE8(g_connectionStatus),
                          BYTE8(g_checksumBits)));
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

    g_checksumBits = ((g_isCurrentTimeValid)      ? 0x01 : 0x00) |
                     ((g_isShutdownIntervalValid) ? 0x02 : 0x00) |
                     ((g_isConnectionStatusValid) ? 0x04 : 0x00);

    g_isCurrentTimeValid = false;
    g_isShutdownIntervalValid = false;
    g_isConnectionStatusValid = false;
    g_isShutdownEventRcvd = false;

    Wire.begin(I2C_ADDR);
    Wire.onReceive(onEvent);
    Wire.onRequest(onRequest);

    powerPin.on();

    uint16_t maxWaitAttemps = MAX_ACTIVE_TIME * 10;
    while ((maxWaitAttemps-- != 0) && (!g_isShutdownEventRcvd))
    {
        delay(100);
    }

    powerPin.off();

    g_restartCounter++;
    g_uptimeDurationMs = static_cast<uint32_t>(millis() - startTimeMs);

    pinMode(PB0, INPUT);
    pinMode(PB1, INPUT);
    pinMode(PB2, INPUT);
    pinMode(PB3, INPUT);

    if (g_isShutdownIntervalValid == true)
    {
        uint16_t shutdownInterval =
            RANGE(g_shutdownInterval, MIN_SLEEP_TIME, MAX_SLEEP_TIME);
        uint16_t shutdownTimeRound =
            RANGE(g_shutdownTimeRound, 0, MAX_SLEEP_TIME);

        wd.powerDown(shutdownInterval, shutdownTimeRound);
    }
    else
    {
        wd.powerDown(DEFAULT_SLEEP_TIME);
    }
}

ISR(WDT_vect, ISR_BLOCK) {}

ISR(SIG_PIN_CHANGE, ISR_BLOCK)
{
    Watchdog &wd = Watchdog::getInstance();
    wd.onInterruptEvent();
}