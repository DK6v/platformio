#include <Arduino.h>

#include "PinLed.h"

namespace app {

// -------------------------------------------------------

PinLed::PinLed(uint8_t pin, bool inverted):
    PinBase(pin),
    mDimm(0), mLastMs(0), mInverted(inverted) {

    pinMode(pin, OUTPUT);
    this->off();
}

void PinLed::on() const {

    pinMode(mPin, OUTPUT);

    if (mDimm == 0) {
        digitalWrite(mPin, (!mInverted) ? HIGH : LOW);
    } else {
        analogWrite(mPin, mDimm);
    }
}

void PinLed::off() const {

    pinMode(mPin, OUTPUT);

    if (mDimm == 0) {
        digitalWrite(mPin, (!mInverted) ? LOW : HIGH);
    } else {
        analogWrite(mPin, 0);
    }
}

void PinLed::setDimm(uint8_t dimm) {
    mDimm = dimm;
}

void PinLed::blink(uint8_t count) {
    blink(NORM, count);
}

void PinLed::blink(Mode mode, uint8_t count) {

    uint32_t intervalMs = duration(mode);

    while (count--) {

        // Wait if several blinks in a row
        uint64_t waitMs = millis() - mLastMs;
        if (waitMs < intervalMs) {
            delay(intervalMs - waitMs);
        }

        this->on();
        delay(intervalMs);
        this->off();
        mLastMs = millis();
    }
}

uint32_t PinLed::duration(Mode mode) {

    switch (mode) {
        case SHORT: return 50;
        case LONG:  return 500;
        
        case NORM: // fall through
        default:    return 100;
    }
}

} // namespace fm