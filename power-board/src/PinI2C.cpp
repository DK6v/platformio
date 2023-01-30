#include <Arduino.h>
#include <Wire.h>

#include "PinI2C.h"

namespace app {

PinI2CListener::PinI2CListener() {}

bool PinI2CListener::begin(uint8_t address, void (*cb)(int)) {

    mAddress = address;

    Wire.begin(mAddress);
    Wire.onReceive(cb);

    return true;
}

bool PinI2CListener::available() {
    return (0 != Wire.available());
}

} // namespace fm