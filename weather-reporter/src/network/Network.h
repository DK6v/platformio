#pragma once

#include "config/Config.h"

namespace app {

class Network {
public:
    Network();
    ~Network() = default;

    enum class Type;
    enum class Status;

    void connect();
    void connect_w_tmo(Type type, uint16_t waitSecs);

    void disconnect();

    Status status();

public:
    enum class Type { SLOW, FAST };
    enum class Status { DISCONNECTED, CONNECTED };

protected:
    void fastConnect();
    void slowConnect();
};

inline static Network network;

}