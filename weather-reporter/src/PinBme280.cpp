#include <Arduino.h>

#include "PinBme280.h"

namespace app {

// -------------------------------------------------------

PinBme280::PinBme280(uint8_t sda, uint8_t scl)
    : mSdaPin(sda),
      mSclPin(scl),
      mAvailable(false),
      temperature(NAN),
      pressure(NAN),
      humidity(NAN) {
}

bool PinBme280::begin(uint8_t address) {

    Wire.begin(mSdaPin, mSclPin);

    delay(1000);
    mAvailable = mSensor.begin(address);

    return mAvailable;
}

void PinBme280::read() {

    readTemperature();
    readPressure();
    readHumidity();
}

float PinBme280::readTemperature() {
    temperature = mSensor.readTemperature();
    return temperature;
}

float PinBme280::readPressure() {
    pressure = mSensor.readPressure();
    return pressure;
}

float PinBme280::readHumidity() {
    humidity = mSensor.readHumidity();
    return humidity;
}

bool PinBme280::available() const {
    return mAvailable;
}

} // namespace fm