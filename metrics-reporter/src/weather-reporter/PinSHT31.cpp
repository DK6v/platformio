#include <Arduino.h>

#include "PinSHT31.h"

namespace app {

// -------------------------------------------------------

PinSHT31::PinSHT31(uint8_t sda, uint8_t scl)
    : mSdaPin(sda),
      mSclPin(scl),
      mAvailable(false),
      temperature(NAN),
      humidity(NAN) {
}

bool PinSHT31::begin(uint8_t address) {

    Wire.begin(mSdaPin, mSclPin);
    mAvailable = mSensor.begin(address);

    return mAvailable;
}

void PinSHT31::read() {

    readTemperature();
    readHumidity();
}

float PinSHT31::readTemperature() {
    temperature = mSensor.readTemperature();
    return temperature;
}

float PinSHT31::readHumidity() {
    humidity = mSensor.readHumidity();
    return humidity;
}

bool PinSHT31::available() const {
    return mAvailable;
}

} // namespace fm