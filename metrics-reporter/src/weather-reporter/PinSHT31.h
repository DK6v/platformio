#pragma once

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>

#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinSHT31 {
public:
    PinSHT31(uint8_t sda, uint8_t scl);
    ~PinSHT31() = default;

    bool begin(uint8_t address = 0x44);
    void read();

    float readTemperature();
    float readHumidity();

    bool available() const;

protected:
    uint8_t mSdaPin;
    uint8_t mSclPin;

    bool mAvailable;
    Adafruit_SHT31 mSensor;

public:
    float temperature;
    float humidity;
};

} // namespace fm