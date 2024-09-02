#include <Arduino.h>

#include "PinCounter.h"

namespace app {
 
PinCounter::PinCounter(uint8_t pin):
    PinBase(pin),
    mCounter(0) {

    pinMode(mPin, OUTPUT);
    
    cli();                          // Disable interrupts during setup
    PCMSK |= (1 << PCINT1);            // Enable interrupt handler (ISR) for our chosen interrupt pin (PCINT1/PB1/pin 6)
    GIMSK |= (1 << PCIE);           // Enable PCINT interrupt in the general interrupt mask
    pinMode(PB1, INPUT_PULLUP); // Set our interrupt pin as input with a pullup to keep it stable
    sei();                          //last line of setup - enable interrupts after setup
}

void PinCounter::onInterruptEvent() {
    ++mCounter;
}

uint8_t PinCounter::get() {
    return mCounter;
}

} // namespace app