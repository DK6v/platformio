#pragma once

#include <Arduino.h>

#include "PinBase.h"

namespace app {

class PinVolts : public PinBase {
public:
    PinVolts(uint8_t pin, float R1, float R2, uint8_t mode = INTERNAL);
    ~PinVolts() = default;

    void setMode(uint8_t mode);

    float read(uint8_t attempts = 1);

private:
    float mR1, mR2;
    float mVRef;
};

} // namespace fm