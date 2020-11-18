#pragma once

#include <utils/ByteBuffer.h>

class AbstractSocket {
public:
    virtual void start() = 0;

    bool send(const void* address, ByteBuffer* packet) {
        return send(address, packet->getBuffer(), packet->getSize());
    };

    virtual bool send(const void* address, const char payload[], unsigned int size) = 0;

    virtual void stop() = 0;

    virtual bool createSocket(Player* player, Server* target) = 0;
};