#pragma once

#include <utils/ByteBuffer.h>

class AbstractSocket {
public:
    virtual void start() = 0;

    void send(const void* address, ByteBuffer* packet) {
        send(address, packet->getBuffer(), packet->getSize());
    };

    virtual void send(const void* address, const char payload[], unsigned short size) = 0;

    virtual void stop() = 0;
};