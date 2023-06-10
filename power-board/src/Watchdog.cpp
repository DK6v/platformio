#include <Arduino.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "Watchdog.h"

volatile bool interruptReceived = true;

namespace app {

  Watchdog::Watchdog()
      : mCalibrationFactor(0L) {
  }

  void Watchdog::onInterruptEvent() {
      mInterruptReceived = true;
  }

  void Watchdog::calibrate(secs_t calibrationInterval) {
      auto startMs = millis();
      this->delay(calibrationInterval);

      mCalibrationFactor = 
        (-1L) * ((static_cast<msec_t>(millis() - startMs) - calibrationInterval * MSEC) /
                 calibrationInterval);
  }

  void Watchdog::calibrate(secs_t interval, secs_t drift) {
    
      if (drift != 0) {

          if ((interval / abs(drift)) < 10 ) {
              setCalibration(rcopysign(10000L /* 10 ms per second */, drift)); 
          }
          else if ((interval / abs(drift)) < 50 ) {
              setCalibration(rcopysign(5000L /* 5 ms per second */, drift));
          }
          else if ((interval / abs(drift)) < 100 ) {
              setCalibration(rcopysign(1000L /* 1 ms per second */, drift));
          }
          else {
              setCalibration(rcopysign(250L /* 1/4 ms per second */, drift)); 
          } 
      }
  }

  void Watchdog::resetCalibration() {
      mCalibrationFactor = 0L;
  }

  Watchdog::usec_t Watchdog::getCalibration() const {
      return mCalibrationFactor;
  }

  void Watchdog::setCalibration(usec_t factor) {
      this->mCalibrationFactor += factor;
  }

  void Watchdog::sleep(secs_t interval) const {

    msec_t cycles =
      ((interval * MSEC) +
       (interval * (mCalibrationFactor / CAL_UNIT)) / (MSEC / CAL_UNIT));

    cycles = cycles - cycles % 250; /* msec */

    // Allow system sleep and set sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);     

    while (cycles > 0) {

      uint8_t wdtcrBits = 0;
      if      (cycles >= 8000) { cycles -= 8000; wdtcrBits = Watchdog::WDTCR_8S; }
      else if (cycles >= 4000) { cycles -= 4000; wdtcrBits = Watchdog::WDTCR_4S; }
      else if (cycles >= 2000) { cycles -= 2000; wdtcrBits = Watchdog::WDTCR_2S; }
      else if (cycles >= 1000) { cycles -= 1000; wdtcrBits = Watchdog::WDTCR_1S; }
      else if (cycles >= 500)  { cycles -= 500;  wdtcrBits = Watchdog::WDTCR_500MS; }
      else                     { cycles -= 250;  wdtcrBits = Watchdog::WDTCR_250MS; }

      WDTCR |= bit(WDCE) | bit(WDE);
      WDTCR = bit(WDIE) | wdtcrBits;
   
      sleep_mode ();

      asm volatile ("WDR");
    }

    WDTCR |= (1<<WDCE) | (1<<WDE);
    WDTCR = 0x00;
  }

  void Watchdog::delay(secs_t interval) {
    
    secs_t cycles = interval + (interval * mCalibrationFactor ) / Watchdog::MSEC;
   
    MCUSR &= ~(bit(WDRF));
    WDTCR |= bit(WDCE) | bit(WDE);
    WDTCR = 0;

    while (cycles) {
  
      uint8_t wdtcrBits = 0;
      if      (cycles >= 8) { cycles -= 8; wdtcrBits = Watchdog::WDTCR_8S; }
      else if (cycles >= 4) { cycles -= 4; wdtcrBits = Watchdog::WDTCR_4S; }
      else if (cycles >= 2) { cycles -= 2; wdtcrBits = Watchdog::WDTCR_2S; }
      else                  { cycles -= 1; wdtcrBits = Watchdog::WDTCR_1S; }

      WDTCR = wdtcrBits; 
      WDTCR |= bit(WDIE);
   
      asm volatile ("WDR");

      mInterruptReceived = false;
      while (mInterruptReceived == false);
    }

    MCUSR &= ~(bit(WDRF));

    WDTCR |= (1<<WDCE) | (1<<WDE);
    WDTCR = 0x00;
  }

  void reset() {
    asm volatile ("WDR");
  }

} // namespace app