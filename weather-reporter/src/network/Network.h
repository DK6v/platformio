#pragma once

#include "config/Config.h"

namespace app {

class Network {
public:
    Network();
    ~Network() = default;

    void connect();

    void fastConnect();
    void slowConnect();

    void disconnect();

    enum class Status;
    Status status();

public:
    enum class Status {
        DISCONNECTED,
        CONNECTED
    };
};

inline static Network network;

}