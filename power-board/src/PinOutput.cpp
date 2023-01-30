#include <Arduino.h>

#include "PinOutput.h"

namespace app {

PinOutput::PinOutput(uint8_t pin):
    PinBase(pin),
    mValue(LOW) {

    pinMode(mPin, OUTPUT);
    digitalWrite(mPin, mValue);
}

void PinOutput::on() {
    mValue = HIGH;
    digitalWrite(mPin, mValue);
}

void PinOutput::off() {
    mValue = LOW;
    digitalWrite(mPin, mValue);
}

void PinOutput::invert() {
    mValue = (mValue == HIGH) ? LOW : HIGH;
    digitalWrite(mPin, mValue);
}

void PinOutput::reset() {
    pinMode(mPin, OUTPUT);
    digitalWrite(mPin, mValue);
}

uint8_t PinOutput::get() {
    return mValue;
}

} // namespace fm