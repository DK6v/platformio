#pragma once

#include <Arduino.h>

namespace app {

class Watchdog {

  using secs_t = signed long;
  using msec_t = signed long;
  using usec_t = signed long;

private:
  Watchdog();

public:
  Watchdog(Watchdog &) = delete;
  Watchdog(Watchdog &&) = delete;
  Watchdog& operator=(const Watchdog &) = delete;
  Watchdog& operator=(const Watchdog &&) = delete;

  ~Watchdog() = default;
  
  static Watchdog& getInstance() {
    static Watchdog wd;
    return wd;
  }

public:  
  void calibrate(secs_t interval = Watchdog::CAL_INTERVAL);
  void calibrate(secs_t interval, secs_t drift);

  void resetCalibration();

  usec_t getCalibration() const;
  void setCalibration(usec_t factor);

  void onInterruptEvent();

  void delay(secs_t interval);
  void sleep(secs_t interval) const;

  void reset();

private:
  template <typename T>
  inline T sign(T val) { return (val > 0) ? (1) : (-1); }
  
  template <typename T, typename S>
  inline T copysign(T val, S sgn) { return (sign(sgn) * sign(val) * (val)); }

  template <typename T, typename S>
  inline T rcopysign(T val, S sgn) { return ((-1) * copysign(val, sgn)); }

public:
  static const secs_t CAL_INTERVAL  = 20L;
  static const msec_t CAL_UNIT      = 100L; // 1/10 seconds

  static const usec_t USEC          = 1000000L;
  static const msec_t MSEC          = 1000L;

  static const uint8_t WDTCR_8S     = bit(WDP3) | bit(WDP0);
  static const uint8_t WDTCR_4S     = bit(WDP3);
  static const uint8_t WDTCR_2S     = bit(WDP2) | bit(WDP1) | bit(WDP0);
  static const uint8_t WDTCR_1S     = bit(WDP2) | bit(WDP1);
  static const uint8_t WDTCR_500MS  = bit(WDP2) | bit(WDP0);
  static const uint8_t WDTCR_250MS  = bit(WDP2);
  static const uint8_t WDTCR_125MS  = bit(WDP1) | bit(WDP0);
  static const uint8_t WDTCR_64MS   = bit(WDP1);
  static const uint8_t WDTCR_32MS   = bit(WDP0);
  static const uint8_t WDTCR_16MS   = 0;

private:
  usec_t mCalibrationFactorUs;
  
  volatile bool mInterruptReceived;
};

} // namespace app