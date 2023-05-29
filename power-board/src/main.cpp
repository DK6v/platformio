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

#define LOW_BYTE(x)   (uint8_t)((x) & 0xFF)
#define HIGH_BYTE(x)  (uint8_t)(((x) >> 8) & 0xFF)

extern volatile bool interruptReceived;

app::PinOutput powerPin(PIN_D4);
app::PinVolts batteryPin(PIN_D3, 10.0, 36.0);

volatile bool powerDownEvent = false;

void onEvent(int size) {

  while(Wire.available()) {

    char c = Wire.read();
    switch(c) {
      case 'P':
        powerDownEvent = true;
        break;
    }
  }
}

void onRequest() {

  uint16_t batteryVolts = (uint16_t)(batteryPin.read(9) * 100);

  Wire.write(HIGH_BYTE(batteryVolts));
  Wire.write(LOW_BYTE(batteryVolts));
}

void setup() {

  app::Watchdog& wd = app::Watchdog::getInstance();
  // wd.calibrate();

  Wire.begin(0x0A);
  Wire.onReceive(onEvent);
  Wire.onRequest(onRequest);

  delay(2000);
}

void loop() {

  app::Watchdog& wd = app::Watchdog::getInstance();
  app::msec startTime = millis();

  powerDownEvent = false;
  powerPin.on();

  app::secs wait = 30;
  while (((wait--) != 0) && (!powerDownEvent)) {
    delay(1000);
  };

  app::msec endTime = millis();
  powerPin.off();

  wd.sleep(600 - (endTime - startTime)/app::SECONDS);
}

ISR ( WDT_vect ) {

  app::Watchdog& wd = app::Watchdog::getInstance();
  wd.onInterruptEvent();
}