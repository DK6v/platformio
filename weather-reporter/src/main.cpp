#include <Arduino.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#include "Checksum.h"
#include "Timer.h"
#include "TimeRFC868.h"
#include "Reporter.h"
#include "PinLed.h"
#include "PinBme280.h"
#include "PinBh1750.h"

#define RANGE(value, min, max) \
    (((value) < (min)) ? (min) : (((value) > (max)) ? (max) : (value)))

#define ROUNDF(x) (((int32_t)((x) * 100.0 + 0.5)) / 100.0)

#define I2C_ADDR 0x0A

#ifdef BUILD_DEBUG
  #define REPORT_NAME "test"
  #define SENSOR_NAME "test"
  #define REPORT_INTERVAL 120
#else
  #define REPORT_NAME "weather"
  #define SENSOR_NAME "bme280"
  #define REPORT_INTERVAL 900
#endif

using namespace app;

WiFiManager wm;
app::Reporter reporter("192.168.0.5", 42001);
app::TimeRFC868 timeRfc868("192.168.0.5", 37);
app::PinBme280 bme(PIN_SDA, PIN_SCL);
app::PinBH1750 light(PIN_SDA, PIN_SCL);

#ifdef BUILD_DEBUG
app::PinLed pinLed(PIN_LED, true);
#endif 

volatile secs_t currentTime = TIME_INVALID;
volatile ticks_t currentTicks = TICKS_INVALID;

volatile secs_t sleepInterval = REPORT_INTERVAL;

void setup() {

#ifdef BUILD_DEBUG
    pinLed.blink(PinLed::LONG, 2);
#endif

    Wire.begin(PIN_SDA, PIN_SCL);

    // Read battery voltage
    Wire.beginTransmission(I2C_ADDR);
    Wire.requestFrom(I2C_ADDR, 10);

    // Read battary voltage
    int16_t batteryVolts = 0;
    int16_t calibration = 0;
    int16_t checksumBits = 0xFF;
    uint32_t boardTime = 0;

    char checksum = 0xFF;

    for (uint8_t wait = 30; wait != 0; --wait, delay(100)) {

        if (Wire.available()) {

            batteryVolts = ((uint16_t)Wire.read());
            batteryVolts |= ((uint16_t)Wire.read()) << 8;

            calibration = (int16_t)Wire.read();
            calibration |= ((int16_t)Wire.read()) << 8;

            boardTime  = ((uint32_t)Wire.read());
            boardTime |= ((uint32_t)Wire.read()) << 8;
            boardTime |= ((uint32_t)Wire.read()) << 16;
            boardTime |= ((uint32_t)Wire.read()) << 24;

            checksumBits = (uint8_t)Wire.read();

            /* Skip checksum */
            checksum = Wire.read();

            break;
        }
    }

    if (checksum == CRC(CRC16(batteryVolts),
                        CRC16(calibration),
                        CRC32(boardTime),
                        CRC8(checksumBits))) {

        checksumBits |= 0x04;
    }
    else {
        batteryVolts = 0;
        calibration = 0;
        checksumBits = 0;
    }

    Wire.endTransmission();

    // Read measurements
    for (uint8_t wait = 5; wait != 0; --wait, delay(200)) {

        Wire.beginTransmission(0x76);
        uint8_t rc = Wire.endTransmission();
        if (rc == 0) {
            bme.begin(0x76);
            bme.read();
            break;
        }
    }

    for (uint8_t wait = 5; wait != 0; --wait, delay(200)) {

        Wire.beginTransmission(0x23);
        uint8_t rc = Wire.endTransmission();
        if (rc == 0) {
            light.begin(0x23);
            light.read();
            break;
        }
    }

    // Send report
    if (wm.autoConnect()) {

        currentTime = timeRfc868.getCurrentTime(REPORT_NAME);
        currentTicks = millis();

        if (currentTime != TIME_INVALID) {
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

        if (!std::isnan(light.lightLevel)) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "light=" + std::to_string(ROUNDF(light.lightLevel));
        }

        if (batteryVolts != 0) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "battery=" + std::to_string(ROUNDF(batteryVolts / 1000.0));
        }

        if (WiFi.isConnected()) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "rssi=" + std::to_string(WiFi.RSSI());
        }

        if (calibration != 0) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "calibration=" + std::to_string(calibration / 100.0);
        }

#ifdef BUILD_DEBUG
        if (boardTime != 0) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "boardTime=" + std::to_string(boardTime);
        }
#endif
        if (checksumBits != 0xFF) {
            fields += (fields.length() != 0) ? "," : "";
            fields += "checksumBits=" + std::to_string(checksumBits);
        }

#ifdef BUILD_DEBUG
        fields += (fields.length() != 0) ? "," : "";
        fields += "debug=true";
#endif
        if (currentTime != TIME_INVALID) {
            fields += (fields.length() != 0) ? " " : "";
            fields += std::to_string(currentTime) + "000000000" /* ns */;
        }

        if (fields.length() != 0) {
            reporter.send(std::string(REPORT_NAME ",sensor=" SENSOR_NAME " ")
                          + fields);
        }
    }

    if (WiFi.isConnected()) {
        WiFi.disconnect(false);
    }

#ifdef BUILD_DEBUG
    pinLed.blink(PinLed::NORM, 2);
    pinLed.blink(PinLed::LONG, 1);
#endif
}

void loop()
{

    Wire.begin(PIN_SDA, PIN_SCL);

    if (currentTime != TIME_INVALID) {

        Wire.beginTransmission(I2C_ADDR);

        app::secs_t delta = (app::secs_t)(millis() - currentTicks) / app::SECONDS;
        currentTime += RANGE(delta, 0, 120);

        Wire.write(I2C_ADDR);
        Wire.write('T');
        Wire.write((currentTime) & 0xFF);
        Wire.write((currentTime >> 8) & 0xFF);
        Wire.write((currentTime >> 16) & 0xFF);
        Wire.write((currentTime >> 24) & 0xFF);

        /* Checksum */
        Wire.write(CRC('T', CRC32(currentTime)));

        Wire.endTransmission();
    }

    while (true) {

        Wire.beginTransmission(I2C_ADDR);

        Wire.write(I2C_ADDR);
        Wire.write('S');
        Wire.write(LOBYTE(REPORT_INTERVAL));
        Wire.write(HIBYTE(REPORT_INTERVAL));

        /* Checksum */
        Wire.write(CRC('S', CRC16(REPORT_INTERVAL)));

        Wire.endTransmission();

        delay(5 * app::SECONDS);
    }
}
