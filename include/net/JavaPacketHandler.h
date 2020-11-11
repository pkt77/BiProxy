#pragma once

#include <utils/ByteBuffer.h>

class Proxy;

class JavaPacketHandler {
private:
    Proxy* proxy;

public:
    JavaPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(const void* address, ByteBuffer* packet) const;

    void disconnect(const void* address) const;
};