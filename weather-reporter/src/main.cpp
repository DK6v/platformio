#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#include "Timer.h"
#include "TimeRFC868.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"

#define LOW_BYTE(x)       (uint8_t)((x) & 0xFF)
#define HIGH_BYTE(x)      (uint8_t)(((x) >> 8) & 0xFF)

#define I2C_ADDR          0x0A

#define REPORT_NAME       "weather"
#define REPORT_INTERVAL   900

WiFiManager wm;
app::Reporter reporter("192.168.0.5", 42001);
app::TimeRFC868 timeRfc868("192.168.0.5", 37);
app::PinBme280 bme(PIN_SDA, PIN_SCL);

unsigned int currentTime = 0;
unsigned int sleepInterval = REPORT_INTERVAL;

void setup() {

    Wire.begin(PIN_SDA, PIN_SCL);

    // Read battery voltage 
    Wire.beginTransmission(I2C_ADDR);
    Wire.requestFrom(I2C_ADDR, 2);

    float battaryVolts = 0.0;
    for (uint8_t wait = 30; wait != 0; --wait, delay(100)) {
    
        if (Wire.available() >= 2) {
            battaryVolts += ((uint16_t)Wire.read());
            battaryVolts += ((uint16_t)Wire.read()) << 8;
            battaryVolts /= 100;
            break;
        }
    }
    Wire.endTransmission();

    // Read measurements
    Wire.beginTransmission(0x76);
    uint8_t rc = Wire.endTransmission();
    if (rc == 0) {
        bme.begin(0x76);
        bme.read();
    }
    
    // Send report
    wm.autoConnect();
  
    currentTime = timeRfc868.getCurrentTime();
    if (currentTime != 0) { 
        currentTime -= 2208988800; // convert to epoch time
    }

    reporter.send(REPORT_NAME ",sensor=bme280"
                  " temp="     + std::to_string(bme.temperature) +
                  ",pressure=" + std::to_string(bme.pressure) +
                  ",humidity=" + std::to_string(bme.humidity) +
                  ",battery="  + std::to_string(battaryVolts) +
                  ((currentTime == 0) ? "" :
                   (",timestamp=" + std::to_string(currentTime))));

  if (WiFi.isConnected()) {
    WiFi.disconnect(false);
  }
}

void loop() {  

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(I2C_ADDR);

  if (currentTime != 0) {
    Wire.write(I2C_ADDR);
    Wire.write("T");
    Wire.write((currentTime)       & 0xFF);
    Wire.write((currentTime >> 8)  & 0xFF);
    Wire.write((currentTime >> 16) & 0xFF);
    Wire.write((currentTime >> 24) & 0xFF);
  }

  while(true) {

    Wire.write(I2C_ADDR);
    Wire.write("S");
    Wire.write(LOW_BYTE(REPORT_INTERVAL));
    Wire.write(HIGH_BYTE(REPORT_INTERVAL));

    Wire.endTransmission();

    delay(1000);
  }
}