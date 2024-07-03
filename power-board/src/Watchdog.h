#pragma once

#include <Arduino.h>

namespace app {

using secs_t = signed long;
using msec_t = signed long;
using usec_t = signed long;

enum : signed long {
    DATETIME_INVALID = ~(1L << (8 * sizeof(signed long) - 1))
};

class Watchdog {

private:
    Watchdog();

public:
    Watchdog(Watchdog &) = delete;
    Watchdog(Watchdog &&) = delete;
    Watchdog &operator=(const Watchdog &) = delete;
    Watchdog &operator=(const Watchdog &&) = delete;

    ~Watchdog() = default;

    static Watchdog &getInstance() {
        static Watchdog wd;
        return wd;
    }

public:
    void onInterruptEvent();

    usec_t getCalibration();
    void setCalibration(usec_t factor);
    void resetCalibration();

    void calibrate(secs_t interval);
    void calibrate(secs_t interval, secs_t drift);

    void setCurrentTime(secs_t epoch, bool bCalibrateTimers = true);
    secs_t datetime();

    void delayInterval(secs_t interval);
    void powerDown(secs_t interval);
    secs_t powerDown(secs_t interval, secs_t round);

private:
    template <typename T>
    inline T sign(T val) { return (val > 0) ? (1) : (-1); }

    template <typename T, typename S>
    inline T copysign(T val, S sgn) { return (sign(sgn) * sign(val) * (val)); }

    template <typename T, typename S>
    inline T rcopysign(T val, S sgn) { return ((-1) * copysign(val, sgn)); }

public:
    static const msec_t MSEC_US   = 1000L;
    static const msec_t SECOND_MS = 1000L;
    static const msec_t MINUTE_MS = 60000L;

    static const uint8_t WDTCR_8S = bit(WDP3) | bit(WDP0);
    static const uint8_t WDTCR_4S = bit(WDP3);
    static const uint8_t WDTCR_2S = bit(WDP2) | bit(WDP1) | bit(WDP0);
    static const uint8_t WDTCR_1S = bit(WDP2) | bit(WDP1);
    static const uint8_t WDTCR_500MS = bit(WDP2) | bit(WDP0);
    static const uint8_t WDTCR_250MS = bit(WDP2);
    static const uint8_t WDTCR_125MS = bit(WDP1) | bit(WDP0);
    static const uint8_t WDTCR_64MS = bit(WDP1);
    static const uint8_t WDTCR_32MS = bit(WDP0);
    static const uint8_t WDTCR_16MS = 0;

private:
    volatile bool mInterruptReceived;
    usec_t mCalibrationFactorUs;
    uint8_t mNiceCalibrationCounter;
    secs_t mBaseDateTimeSecs;
    msec_t mBaseDateTimeMillis;
    secs_t mLastCalibrationTime;
};

} // namespace app