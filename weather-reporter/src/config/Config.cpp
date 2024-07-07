#include <EEPROM.h>
#include <string>

#include "Config.h"
#include "Console.h"

namespace app {

Config::Config()
    : mParameters() {

    add(new ConfigParameter<char>(0, 0xA7));
}

Config& Config::write(ByteBuffer& buffer) {

    ByteBuffer::iterator it = buffer.begin();

    for (auto & [ key, parameter]: mParameters) {

        if (!it.isValid()) {
            console.log("Write failed, invalid iterator");
            break;
        }

        it = parameter->write(it);
    }

    buffer.commit();

    return *this;
}

Config& Config::read(ByteBuffer& buffer) {

    ByteBuffer::iterator it = buffer.begin();

    for (auto & [ key, parameter]: mParameters) {

        if (!it.isValid()) {
            break;
        }

        it = parameter->read(it);
    }

    return *this;
}

}
