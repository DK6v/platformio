#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#include "Timer.h"
#include "TimeRFC868.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define NBYTE(n, x) ((uint8_t)(((x) >> (8 * ((n)-1))) & 0xFF))
#define LOBYTE(x) NBYTE(1, x)
#define HIBYTE(x) NBYTE(2, x)

#define CRC16(x) (NBYTE(1, x) ^ NBYTE(2, x))
#define CRC32(x) (NBYTE(1, x) ^ NBYTE(2, x) ^ NBYTE(3, x) ^ NBYTE(4, x))

#define ROUNDF(x) (((int32_t)((x) * 100.0 + 0.5)) / 100.0)

#define I2C_ADDR 0x0A

#define REPORT_NAME "weather"
#define REPORT_INTERVAL 900

WiFiManager wm;
app::Reporter reporter("192.168.0.5", 42001);
app::TimeRFC868 timeRfc868("192.168.0.5", 37);
app::PinBme280 bme(PIN_SDA, PIN_SCL);

volatile unsigned long currentTime = 0;
volatile unsigned long currentTicks = 0;

volatile unsigned long sleepInterval = REPORT_INTERVAL;

void setup() {

    Wire.begin(PIN_SDA, PIN_SCL);

    // Read battery voltage
    Wire.beginTransmission(I2C_ADDR);
    Wire.requestFrom(I2C_ADDR, 6);

    // Read battary voltage
    int16_t batteryVolts = 0;
    int16_t calibration = 0;
    int16_t checksumBits = 0xFF;

    char checksum = 0xFF;

    for (uint8_t wait = 30; wait != 0; --wait, delay(100)) {

        if (Wire.available()) {

            batteryVolts = ((uint16_t)Wire.read());
            batteryVolts |= ((uint16_t)Wire.read()) << 8;

            calibration = (int16_t)Wire.read();
            calibration |= ((int16_t)Wire.read()) << 8;

            checksumBits = (uint8_t)Wire.read();

            /* Skip checksum */
            checksum = Wire.read();

            break;
        }
   }

    if (checksum == CRC16(batteryVolts ^ calibration ^ (uint16_t)checksumBits)) {

        checksumBits |= 0x04;
    }
    else {
        batteryVolts = 0;
        calibration = 0;
        checksumBits = 0;
    }

    Wire.endTransmission();

    // Read measurements
    for (uint8_t wait = 30; wait != 0; --wait, delay(100)) {

        Wire.beginTransmission(0x76);
        uint8_t rc = Wire.endTransmission();
        if (rc == 0) {
            bme.begin(0x76);
            bme.read();
            break;
        }
    }

    // Send report
    if (wm.autoConnect()) {

        currentTime = timeRfc868.getCurrentTime();
        currentTicks = millis();

        if (currentTime != 0) {
            currentTime -= 2208988800; // Convert to unix epoch (1900 -> 1970)
        }

        std::string fields;

        if (!std::isnan(bme.temperature)) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "temp=" + std::to_string(ROUNDF(bme.temperature));
        }

        if (!std::isnan(bme.pressure)) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "pressure=" + std::to_string(ROUNDF(bme.pressure));
        }

        if (!std::isnan(bme.humidity)) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "humidity=" + std::to_string(ROUNDF(bme.humidity));
        }

        if (batteryVolts != 0) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "battery=" + std::to_string(ROUNDF(batteryVolts / 1000.0));
        }

        if (calibration != 0) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "calibration=" + std::to_string(calibration / 100.0);
        }

        if (checksumBits != 0xFF) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "checksumBits=" + std::to_string(checksumBits);
        }

        if (!std::isnan(currentTime)) {
            fields += (fields.length() != 0) ? " " : "";
            fields += std::to_string(currentTime) + "000000000" /* ns */;
        }

        if (fields.length() != 0) {
            reporter.send(std::string(REPORT_NAME ",sensor=bme280 ") + fields);
        }
    }

    if (WiFi.isConnected()) {
        WiFi.disconnect(false);
    }
}

void loop()
{

    Wire.begin(PIN_SDA, PIN_SCL);

    if (currentTime != 0) {

        Wire.beginTransmission(I2C_ADDR);

        app::secs delta = (app::secs)(millis() - currentTicks) / app::SECONDS;
        currentTime += RANGE(delta, 0, 60 /* seconds */);

        Wire.write(I2C_ADDR);
        Wire.write('T');
        Wire.write((currentTime)&0xFF);
        Wire.write((currentTime >> 8) & 0xFF);
        Wire.write((currentTime >> 16) & 0xFF);
        Wire.write((currentTime >> 24) & 0xFF);

        /* Checksum */
        Wire.write('T' ^
                   NBYTE(1, currentTime) ^
                   NBYTE(2, currentTime) ^
                   NBYTE(3, currentTime) ^
                   NBYTE(4, currentTime));

        Wire.endTransmission();
    }

    while (true) {

        Wire.beginTransmission(I2C_ADDR);

        Wire.write(I2C_ADDR);
        Wire.write('S');
        Wire.write(LOBYTE(REPORT_INTERVAL));
        Wire.write(HIBYTE(REPORT_INTERVAL));

        /* Checksum */
        Wire.write('S' ^ CRC16(REPORT_INTERVAL));

        Wire.endTransmission();

        delay(5 * app::SECONDS);
    }
}