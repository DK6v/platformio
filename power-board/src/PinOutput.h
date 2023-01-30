#pragma once

#include <Arduino.h>

#include "PinBase.h"

namespace app {

class PinOutput : public PinBase {
public:
    PinOutput(uint8_t pin);
    ~PinOutput() = default;

    void on();
    void off();
    void reset();
    void invert();

    uint8_t get();

private:
    uint8_t mValue;
};

} // namespace fm