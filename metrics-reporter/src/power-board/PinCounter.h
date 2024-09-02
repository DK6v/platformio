#pragma once

#include <Arduino.h>

#include "PinBase.h"

namespace app {

class PinCounter : public PinBase {
public:
    explicit PinCounter(uint8_t pin);
    ~PinCounter() = default;

    uint8_t get();

    void onInterruptEvent();

private:
    uint8_t mCounter;
};

} // namespace fm