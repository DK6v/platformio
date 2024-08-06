#pragma once

#include <Arduino.h>
#include <EEPROM.h>

#include "Console.h"
#include "ByteBuffer.h"

namespace app {

class StorageEeprom : public ByteBuffer {
public:
    StorageEeprom(unsigned short size);
    ~StorageEeprom() = default;

    const char read(unsigned short index);
    void write(unsigned short index, const char value);

    void commit();
    unsigned short size();

private:
    unsigned short mSize;
};

} // namespace