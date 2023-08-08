#pragma once

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinBme280 {
public:
    PinBme280(uint8_t sda, uint8_t scl);
    ~PinBme280() = default;

    bool begin(uint8_t address = 0x76);
    void read();

    float readTemperature();
    float readPressure(void);
    float readHumidity(void);

    bool available() const;

protected:
    uint8_t mSdaPin;
    uint8_t mSclPin;

    bool mAvailable;
    Adafruit_BME280 mSensor;

public:
    float temperature;
    float pressure;
    float humidity;
};

} // namespace fm