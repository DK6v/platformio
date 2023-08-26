#include <Arduino.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "Watchdog.h"

volatile bool interruptReceived = true;

namespace app {

  Watchdog::Watchdog()
      : mCalibrationFactorUs(0L) {
  }

  void Watchdog::onInterruptEvent() {
      mInterruptReceived = true;
  }

  void Watchdog::calibrate(secs_t calibrationInterval) {
      auto startMs = millis();
      this->delay(calibrationInterval);

      mCalibrationFactorUs = 
        (-1L) * ((static_cast<msec_t>(millis() - startMs) - calibrationInterval * MSEC) /
                 calibrationInterval);
  }

  void Watchdog::calibrate(secs_t interval, secs_t drift) {
    
      if (drift != 0) {

          /* drift >10% => 10 ms per second (1%) */
          if ((interval / abs(drift)) < 10) {
              setCalibration(rcopysign(10000L /* us */, drift)); 
          }
          /* drift >5% => 5 ms per second (0.5%) */
          else if ((interval / abs(drift)) < 20 ) {
              setCalibration(rcopysign(5000L /* us */, drift));
          }
          /* drift >2.5% => 2.5 ms per second (0.25%) */
          else if ((interval / abs(drift)) < 40 ) {
              setCalibration(rcopysign(2500L /* us */, drift));
          }
          /* drift >1% => 1 ms per second (0.1%) */
          else if ((interval / abs(drift)) < 100 ) {
              setCalibration(rcopysign(1000L /* us */, drift));
          }
          /* drift >0.5% => 0.5 ms per second (0.05%) */
          else if ((interval / abs(drift)) < 200 ) {
              setCalibration(rcopysign(500L /* us */, drift));
          }
          /* drift >0.25% => 0.25 ms per second (0.025%) */
          else if ((interval / abs(drift)) < 400 ) {
              setCalibration(rcopysign(250L /* us */, drift));
          }
          /* drift >0.125% => 0.125 ms per second (0.0125%) */
          else if ((interval / abs(drift)) < 800 ) {
              setCalibration(rcopysign(125L /* us */, drift));
          }
          /* drift <0.125% => 0.05 ms per second (0.005%) */
          else {
              setCalibration(rcopysign(50L /* us */, drift)); 
          } 
      }
  }

  void Watchdog::resetCalibration() {
      mCalibrationFactorUs = 0L;
  }

  Watchdog::usec_t Watchdog::getCalibration() const {
      return mCalibrationFactorUs;
  }

  void Watchdog::setCalibration(usec_t factor) {
      this->mCalibrationFactorUs += factor;
  }

  void Watchdog::sleep(secs_t interval) const {

    msec_t cyclesMs = (interval * MSEC) + (interval * mCalibrationFactorUs) / MSEC;

    cyclesMs = cyclesMs - cyclesMs % 250; /* msec */

    // Allow system sleep and set sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);     

    while (cyclesMs > 0) {

      uint8_t wdtcrBits = 0;
      if      (cyclesMs >= 8000) { cyclesMs -= 8000; wdtcrBits = Watchdog::WDTCR_8S; }
      else if (cyclesMs >= 4000) { cyclesMs -= 4000; wdtcrBits = Watchdog::WDTCR_4S; }
      else if (cyclesMs >= 2000) { cyclesMs -= 2000; wdtcrBits = Watchdog::WDTCR_2S; }
      else if (cyclesMs >= 1000) { cyclesMs -= 1000; wdtcrBits = Watchdog::WDTCR_1S; }
      else if (cyclesMs >= 500)  { cyclesMs -= 500;  wdtcrBits = Watchdog::WDTCR_500MS; }
      else                       { cyclesMs -= 250;  wdtcrBits = Watchdog::WDTCR_250MS; }

      WDTCR |= bit(WDCE) | bit(WDE);
      WDTCR = bit(WDIE) | wdtcrBits;
   
      sleep_mode ();

      asm volatile ("WDR");
    }

    WDTCR |= (1<<WDCE) | (1<<WDE);
    WDTCR = 0x00;
  }

  void Watchdog::delay(secs_t interval) {
    
    secs_t cyclesMs = interval + (interval * mCalibrationFactorUs ) / Watchdog::MSEC;
   
    MCUSR &= ~(bit(WDRF));
    WDTCR |= bit(WDCE) | bit(WDE);
    WDTCR = 0;

    while (cyclesMs) {
  
      uint8_t wdtcrBits = 0;
      if      (cyclesMs >= 8) { cyclesMs -= 8; wdtcrBits = Watchdog::WDTCR_8S; }
      else if (cyclesMs >= 4) { cyclesMs -= 4; wdtcrBits = Watchdog::WDTCR_4S; }
      else if (cyclesMs >= 2) { cyclesMs -= 2; wdtcrBits = Watchdog::WDTCR_2S; }
      else                    { cyclesMs -= 1; wdtcrBits = Watchdog::WDTCR_1S; }

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