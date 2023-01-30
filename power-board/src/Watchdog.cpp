#include <Arduino.h>

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "Watchdog.h"

volatile bool interruptReceived = true;

namespace app {

  Watchdog::Watchdog() : 
    mCalibrateMsecs(0) {

    // wdt_reset();
    // wdt_disable();
  }

  void Watchdog::onInterruptEvent() {
      mInterruptReceived = true;

      // wdt_reset();
      // wdt_disable();
  }

  void Watchdog::calibrate() {
      auto startMs = millis();
      this->delay(Watchdog::CALIBRATION_INTERVAL);

      mCalibrateMsecs = 
        (-1) * ((static_cast<msec>(millis() - startMs) -
                 Watchdog::CALIBRATION_INTERVAL * Watchdog::MSEC_IN_SECOND) /
                (Watchdog::CALIBRATION_INTERVAL));
  }

  void Watchdog::sleep(secs interval) {

      secs cycles = interval + (interval * mCalibrateMsecs) / Watchdog::MSEC_IN_SECOND;

      // Allow system sleep and set sleep mode
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_enable ();

      // Turn power off to ADC, TIMER 1 and 2, Serial Interface, etc.
      power_all_disable ();

      /* Clear the reset flag on the MCUSR, the WDRF bit (bit 3). */
      MCUSR &= ~(bit(WDRF));

      /* WDE can only be cleared if the WDCE bit has logic level one */ 
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

        sleep_mode();
      }
    
      MCUSR &= ~(bit(WDRF));

      WDTCR |= (1<<WDCE) | (1<<WDE);
      WDTCR = 0x00;

      power_all_enable ();
      sleep_disable ();
  }

  void Watchdog::delay(secs interval) {
       
    secs cycles = interval + (interval * mCalibrateMsecs) / Watchdog::MSEC_IN_SECOND;
   
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