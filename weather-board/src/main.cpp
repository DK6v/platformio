#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#include "Timer.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"

WiFiManager wm;
app::Reporter reporter("192.168.0.5", 42003);

app::PinBme280 bme(PIN_SDA, PIN_SCL);

void setup() {

  delay(5000);
 
  while(!bme.begin(0x76)) {
    delay(1000);
  }

  wm.autoConnect();
}

void loop() {  

  float battaryVolts = 0.0;

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(0x0A);

  Wire.requestFrom(0x0A, 2);

  app::secs wait = 10; 
  while((wait--) != 0) {
    
    if (Wire.available() >= 2) {

      battaryVolts += ((uint8_t)Wire.read()) << 8;
      battaryVolts += ((uint8_t)Wire.read());
      battaryVolts /= 100;

      break;
    }

    delay(1000);
  }

  if (bme.available()) {

    bme.read();
    reporter.send("weather,sensor=bme280"
                  " temp="     + std::to_string(bme.temperature) +
                  ",pressure=" + std::to_string(bme.pressure) +
                  ",humidity=" + std::to_string(bme.humidity) +
                  ",battery="  + std::to_string(battaryVolts));
  }

  while(true) {

    delay(3000);
    
    Wire.write("P");
    Wire.endTransmission();
  }
}