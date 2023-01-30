#pragma once

#include <Arduino.h>

namespace app {

class Watchdog {

using msec=signed int;
using secs=signed int;

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
  void calibrate();
  void onInterruptEvent();

  void delay(secs interval);
  void sleep(secs interval);

  void reset();

public:
  static const signed int MSEC_IN_SECOND       = 1000U;
  static const signed int CALIBRATION_INTERVAL = 2U;

  static const uint8_t WDTCR_8S    = bit(WDP3) | bit(WDP0);
  static const uint8_t WDTCR_4S    = bit(WDP3);
  static const uint8_t WDTCR_2S    = bit(WDP2) | bit(WDP1) | bit(WDP0);
  static const uint8_t WDTCR_1S    = bit(WDP2) | bit(WDP1);
  static const uint8_t WDTCR_500MS = bit(WDP2) | bit(WDP0);
  static const uint8_t WDTCR_250MS = bit(WDP2);
  static const uint8_t WDTCR_125MS = bit(WDP1) | bit(WDP0);
  static const uint8_t WDTCR_64MS  = bit(WDP1);
  static const uint8_t WDTCR_32MS  = bit(WDP0);
  static const uint8_t WDTCR_16MS  = 0;

private:
  msec mCalibrateMsecs;
  
  volatile bool mInterruptReceived;
};

} // namespace app