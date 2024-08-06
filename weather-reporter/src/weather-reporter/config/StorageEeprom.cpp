#include <Arduino.h>
#include <EEPROM.h>

#include "Console.h"
#include "StorageEeprom.h"

namespace app {

StorageEeprom::StorageEeprom(unsigned short size)
    : mSize(size) {

    EEPROM.begin(size);
};

unsigned short StorageEeprom::size() {
    return mSize;
}

const char StorageEeprom::read(unsigned short index) {
    return EEPROM.read(index);
}

void StorageEeprom::write(unsigned short index, const char value) {
    if (EEPROM.read(index) != value) {
        EEPROM.write(index, value);
    }
}

void StorageEeprom::commit() {
    EEPROM.commit();
}

} // namespace