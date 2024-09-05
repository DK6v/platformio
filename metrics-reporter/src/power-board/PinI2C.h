#pragma once

#include <Arduino.h>

#include "PinBase.h"

namespace app {

// -------------------------------------------------------

class PinI2CListener {
private:
    PinI2CListener();

public:
    PinI2CListener(PinI2CListener &) = delete;
    PinI2CListener(PinI2CListener &&) = delete;
    PinI2CListener& operator=(const PinI2CListener &) = delete;
    PinI2CListener& operator=(const PinI2CListener &&) = delete;

    ~PinI2CListener() = default;

    static PinI2CListener& getInstance() {
        static PinI2CListener listener;
        return listener;
    }

    bool begin(uint8_t address, void (*cb)(int));

    bool available();

private:
    uint8_t mAddress;
};

} // namespace fm