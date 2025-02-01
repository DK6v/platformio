#include <Arduino.h>

#include "PinBME280.h"

namespace app {

// -------------------------------------------------------

PinBME280::PinBME280(uint8_t sda, uint8_t scl)
    : mSdaPin(sda),
      mSclPin(scl),
      mAvailable(false),
      temperature(NAN),
      pressure(NAN),
      humidity(NAN) {
}

bool PinBME280::begin(uint8_t address) {

    Wire.begin(mSdaPin, mSclPin);
    mAvailable = mSensor.begin(address);

    return mAvailable;
}

void PinBME280::read() {

    readTemperature();
    readPressure();
    readHumidity();
}

float PinBME280::readTemperature() {
    temperature = mSensor.readTemperature();
    return temperature;
}

float PinBME280::readPressure() {
    pressure = mSensor.readPressure();
    return pressure;
}

float PinBME280::readHumidity() {
    humidity = mSensor.readHumidity();
    return humidity;
}

bool PinBME280::available() const {
    return mAvailable;
}

} // namespace fm