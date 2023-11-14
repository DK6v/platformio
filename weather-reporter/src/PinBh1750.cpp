#include <Arduino.h>

#include "PinBh1750.h"

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

    delay(1000);
    mAvailable = mSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

    return mAvailable;
}

float PinBH1750::readLightLevel() {

    if (available()) {

        delay(1000);
        lightLevel = mSensor.readLightLevel();

        return (lightLevel >= 0.0) ? lightLevel : NAN;
    }

    return NAN;
}

bool PinBH1750::available() const {

    return mAvailable;
}

void PinBH1750::read() {

    readLightLevel();
}

} // namespace app