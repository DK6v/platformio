#include <Arduino.h>

#include "SortHelper.h"
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

float PinVolts::read(uint8_t attempts) {

    float readValue = 0.0;

    (void)analogRead(mPin);

    switch (attempts) {
        case 0: // fall through
        case 1: {
            readValue = (float)analogRead(mPin);
            break;
        }

        case 2: {
            readValue = (float)analogRead(mPin);
            break;
        }

        default: {

            float readValues[attempts] = {};
            uint8_t numOfReadings = 0;

            for (uint8_t ix = 0; ix < attempts; ++ix) {
                readValues[ix] = (float)analogRead(mPin);
            }

            app::bsort(readValues, attempts);

            for (uint8_t ix = (attempts / 3); ix < (attempts - (attempts / 3)); ++ix)
            {
                readValue += readValues[ix];
                ++numOfReadings;
            }
            readValue /= numOfReadings;

            break;
        }
    }

    return ((readValue * mVRef) / 1024) / (mR1 / (mR1 + mR2));
}

} // namespace fm