#pragma once

class Client {
public:
    Client() {}

    virtual int connect(const char *host, uint16_t port) = 0;
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
    virtual void flush() = 0;
    virtual void stop() = 0;
};