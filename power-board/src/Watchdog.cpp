#include <Arduino.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "Watchdog.h"

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

volatile bool interruptReceived = true;

namespace app {

    Watchdog::Watchdog()
        : mCalibrationFactorUs(0L),
          mNiceCalibrationCounter(0),
          mBaseDateTimeSecs(DATETIME_INVALID),
          mBaseDateTimeMillis(DATETIME_INVALID),
          mLastCalibrationTime(DATETIME_INVALID)
    {}

    void Watchdog::onInterruptEvent() {
        mInterruptReceived = true;
    }

    void Watchdog::calibrate(secs_t calibrationInterval) {

        auto startMs = millis();
        this->delay(calibrationInterval);

        mCalibrationFactorUs =
            (-1L) * ((static_cast<msec_t>(millis() - startMs) - calibrationInterval * MSEC_US) /
                     calibrationInterval);
    }

    void Watchdog::calibrate(secs_t interval, secs_t drift) {

        if (drift != 0) {

            float percent = (100.0 * (float)(abs(drift))) / (float)interval;

            /* drift >5% => 30 ms per second (2.5%) */
            if (percent >= 5.0) {
                setCalibration(getCalibration() +
                               rcopysign(25000L /* us */, drift));
            }
            /* drift >2.5% => 5 ms per second (1.25%) */
            else if (percent >= 2.5) {
                setCalibration(getCalibration() +
                               rcopysign(12500L /* us */, drift));
            }
            /* drift >1% => 5.0 ms per second (1.0%) */
            else if (percent >= 1.0) {
                setCalibration(getCalibration() +
                               rcopysign(5000L /* us */, drift));
            }
            /* drift >0.5% => 2.5 ms per second (0.25%) */
            else if (percent >= 0.5) {
                setCalibration(getCalibration() +
                               rcopysign(2500L /* us */, drift));
            }
            /* drift >0.25% => 1 ms per second (0.1%) */
            else if (percent >= 0.25) {
                setCalibration(getCalibration() +
                               rcopysign(1000L /* us */, drift));
            }
            /* drift >0.125% => 0.5...0.05 ms per second (0.02%) */
            else if (percent >= 0.125) {
                setCalibration(getCalibration() +
                               rcopysign(500L / RANGE(mNiceCalibrationCounter, 1, 10), drift));
            }
            /* drift <0.1% => 0.2...0.01 ms per second (0.01%) */
            else {
                setCalibration(getCalibration() + rcopysign(250L / RANGE(mNiceCalibrationCounter, 1, 10), drift));
            }

            if (mNiceCalibrationCounter > 1) {

                mNiceCalibrationCounter -= 1;
            }
        }
        else {

            if (mNiceCalibrationCounter < 10) {

                mNiceCalibrationCounter += 1;
            }
        }
    }

    usec_t Watchdog::getCalibration() {

        return mCalibrationFactorUs;
    }

    void Watchdog::setCalibration(usec_t factor) {

        this->mCalibrationFactorUs = RANGE(factor, -250000, 250000);
    }

    void Watchdog::setCurrentTime(secs_t epoch, bool bCalibrateTimers) {

        if (bCalibrateTimers == true) {

            secs_t currentTime = datetime();

            if ((currentTime != DATETIME_INVALID) &&
                (mLastCalibrationTime != DATETIME_INVALID)) {


                calibrate(currentTime - mLastCalibrationTime, epoch - currentTime);
            }

            mLastCalibrationTime = epoch;
        }

        mBaseDateTimeSecs = epoch;
        mBaseDateTimeMillis = millis();
    }

    secs_t Watchdog::datetime() {

        if (mBaseDateTimeSecs != DATETIME_INVALID) {

            return mBaseDateTimeSecs + (millis() - mBaseDateTimeMillis) / 1000;
        }

        return DATETIME_INVALID;
    }

    void Watchdog::powerDown(secs_t interval) {

        volatile msec_t cyclesMs = interval * SECOND_MS;

        // Apply calibration        
        cyclesMs += (interval * mCalibrationFactorUs) / MSEC_US;

        cyclesMs = RANGE(cyclesMs, 0, 60 * MINUTE_MS);

        mBaseDateTimeSecs = datetime();

        // Allow system sleep and set sleep mode
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);

        sleep_enable();

        while (cyclesMs > 0) {

            volatile uint8_t wdtcrBits = 0;

            if (cyclesMs >= 8000)       { cyclesMs -= 8000; wdtcrBits = Watchdog::WDTCR_8S; }
            else if (cyclesMs >= 4000)  { cyclesMs -= 4000; wdtcrBits = Watchdog::WDTCR_4S; }
            else if (cyclesMs >= 2000)  { cyclesMs -= 2000; wdtcrBits = Watchdog::WDTCR_2S; }
            else if (cyclesMs >= 1000)  { cyclesMs -= 1000; wdtcrBits = Watchdog::WDTCR_1S; }
            else if (cyclesMs >= 500)   { cyclesMs -= 500; wdtcrBits = Watchdog::WDTCR_500MS; }
            else if (cyclesMs >= 250)   { cyclesMs -= 250; wdtcrBits = Watchdog::WDTCR_250MS; }
            else                        { break; }

            WDTCR |= bit(WDCE) | bit(WDE);
            WDTCR = bit(WDIE) | wdtcrBits;

            sleep_mode();
        }

        WDTCR |= (1 << WDCE) | (1 << WDE);
        WDTCR = 0x00;

        sleep_disable();

        if (mBaseDateTimeSecs != DATETIME_INVALID) {

            mBaseDateTimeSecs += interval;
            mBaseDateTimeMillis = millis();
        }
    }

    secs_t Watchdog::powerDown(secs_t interval, secs_t round) {

        secs_t currentTime = datetime();
        secs_t timeout = interval;

        if (currentTime != DATETIME_INVALID) {

            secs_t nextTime = currentTime + interval;
            secs_t shift = nextTime % round;

            /* 1: ---r-----C------------N----------r-----------
             *       |     |<---------->|          | : interval
             *       |<--------------------------->| : round
             *       |<---------------->|          | : shift
             *             |<=====================>| : TIMEOUT
             */
            if (currentTime > (nextTime - shift)) {

                timeout = interval + (round - shift);
            }
            /* 2: ---C---r----------N----r---------------r----
             *       |<------------>|    |          : interval
             *       |   |<------------->|          : round
             *       |   |<-------->|    |          : shift
             *       |<=================>|          : TIMEOUT
             */
            else if (shift > round / 2) {

                timeout = interval + (round - shift);
            }
            /* 3: ---C----------r---N-----------r-------------
             *       |<------------>|    |          : interval
             *       |          |<------------->|   : round
             *       |          |<->|    |          : shift
             *       |<=================>|          : TIMEOUT
             */
            else {

                timeout = interval - shift;
            }
        }

        powerDown(timeout);

        return timeout;
    }

    void Watchdog::delay(secs_t interval) {

        secs_t cyclesMs = interval + (interval * mCalibrationFactorUs) / MSEC_US;

        MCUSR &= ~(bit(WDRF));
        WDTCR |= bit(WDCE) | bit(WDE);
        WDTCR = 0;

        while (cyclesMs) {

            uint8_t wdtcrBits = 0;
            if (cyclesMs >= 8)      { cyclesMs -= 8; wdtcrBits = Watchdog::WDTCR_8S; }
            else if (cyclesMs >= 4) { cyclesMs -= 4; wdtcrBits = Watchdog::WDTCR_4S; }
            else if (cyclesMs >= 2) { cyclesMs -= 2; wdtcrBits = Watchdog::WDTCR_2S; }
            else                    { cyclesMs -= 1; wdtcrBits = Watchdog::WDTCR_1S; }

            WDTCR = wdtcrBits;
            WDTCR |= bit(WDIE);

            asm volatile("WDR");

            mInterruptReceived = false;
            while (mInterruptReceived == false) {}
        }

        MCUSR &= ~(bit(WDRF));

        WDTCR |= (1 << WDCE) | (1 << WDE);
        WDTCR = 0x00;
    }
} // namespace app