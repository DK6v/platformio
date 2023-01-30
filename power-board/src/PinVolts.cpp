#include <Arduino.h>

#include "PinVolts.h"

namespace app {

PinVolts::PinVolts(uint8_t pin, float R1, float R2, uint8_t mode) :
    PinBase(pin),
    mR1(R1), mR2(R2), mVRef(1.1) {

    setMode(mode);
}

void PinVolts::setMode(uint8_t mode) {

    pinMode(mPin, INPUT);
    analogReference(mode);

    switch(mode) {
        case INTERNAL1V1:  { mVRef = 1.1;  break; }        
        case INTERNAL2V56: { mVRef = 2.56; break; }
        
        default:
            mVRef = 5.0;
            break;
    }
}


float PinVolts::read() {

    float value = (float)analogRead(mPin);

    return ((value * mVRef) / 1024) / (mR1 / (mR1 + mR2));
}

} // namespace fm