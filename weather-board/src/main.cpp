#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#include "Timer.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"

WiFiManager wm;
app::Reporter reporter("192.168.0.5", 42002);

app::PinBme280 bme(PIN_SDA, PIN_SCL);

void setup() {

  delay(1000);

  Wire.begin(PIN_SDA, PIN_SCL);

  // Read battery voltage 
  Wire.beginTransmission(0x0A);
  Wire.requestFrom(0x0A, 2);

  float battaryVolts = 0.0;

  app::secs wait = 5; 
  while((wait--) != 0) {
    
    if (Wire.available() >= 2) {

      battaryVolts += ((uint8_t)Wire.read()) << 8;
      battaryVolts += ((uint8_t)Wire.read());
      battaryVolts /= 100;

      break;
    }

    delay(1000);
  }

  // Read measurements
  while(!bme.begin(0x76)) {
    delay(1000);
  }
  bme.read();

  // Send report
  wm.autoConnect();
  reporter.send("weather,sensor=bme280"
                  " temp="     + std::to_string(bme.temperature) +
                  ",pressure=" + std::to_string(bme.pressure) +
                  ",humidity=" + std::to_string(bme.humidity) +
                  ",battery="  + std::to_string(battaryVolts));
}

void loop() {  

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(0x0A);

  while(true) {
   
    Wire.write("P");
    Wire.endTransmission();

    delay(1000);
  }
}