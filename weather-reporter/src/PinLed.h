#pragma once

#include <Arduino.h>

#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinLed : public PinBase {
public:
    enum Mode { NORM, SHORT, LONG };
public:
    PinLed(uint8_t pin, bool inverted = false);
    ~PinLed() = default;

    void setDimm(uint8_t dimm);

    void on() const;
    void off() const;

    void blink(uint8_t count);
    void blink(Mode mode = NORM, uint8_t count = 1);

private:
    uint32_t duration(Mode mode);

private:
    uint8_t mDimm;
    uint64_t mLastMs;
    bool mInverted;
};

} // namespace fm