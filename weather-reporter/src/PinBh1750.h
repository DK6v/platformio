#pragma once

#include <Arduino.h>

#include <BH1750.h>
#include <Wire.h>

#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinBH1750 {
public:
    PinBH1750(uint8_t sda, uint8_t scl);
    ~PinBH1750() = default;

    bool begin(uint8_t address = 0x23);
    bool available() const;
    void read();

    float readLightLevel();

protected:
    uint8_t mSdaPin;
    uint8_t mSclPin;

    bool mAvailable;
    BH1750 mSensor;

public:
    float lightLevel;
};

} // namespace fm