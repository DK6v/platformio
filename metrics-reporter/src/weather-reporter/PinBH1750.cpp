#include <Arduino.h>

#include "PinBH1750.h"

namespace app {

// -------------------------------------------------------

PinBH1750::PinBH1750(uint8_t sda, uint8_t scl)
    : mSdaPin(sda),
      mSclPin(scl),
      mAvailable(false),
      lightLevel(NAN) {}

bool PinBH1750::begin(uint8_t address) {

    Wire.begin(mSdaPin, mSclPin);

    (void)address;

    mAvailable = mSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, address);

    return mAvailable;
}

float PinBH1750::readLightLevel() {

    for (uint8_t wait = 20; wait != 0; --wait, delay(50)) {

        if (available()) {

            lightLevel = mSensor.readLightLevel();
            return (lightLevel >= 0.0) ? lightLevel : NAN;
        }
    }

    return NAN;
}

bool PinBH1750::available() {

    return mSensor.measurementReady();
}

void PinBH1750::read() {

    readLightLevel();
}

} // namespace app