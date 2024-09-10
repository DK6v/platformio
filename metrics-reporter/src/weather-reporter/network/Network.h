#pragma once

#include "config/Config.h"

namespace app {

class Network {
public:
    Network();
    ~Network() = default;

    enum class Type;
    enum class Status;

    Status connect(Network::Type type);
    Status connect_w_tmo(Type type, uint16_t waitSecs);

    void disconnect();

    Status status();

public:
    enum class Type { SLOW, FAST };
    enum class Status { UNDEFINED, DISCONNECTED, CONNECTING, CONNECTED, FAILED };

protected:
    Status fastConnect();
    Status slowConnect();
};

inline static Network network;

}