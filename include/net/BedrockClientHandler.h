#pragma once

#include <utils/ByteBuffer.h>

class Proxy;

class BedrockClientHandler {
private:
    Proxy* proxy;

public:
    BedrockClientHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(const void* socket, unsigned char address[], unsigned short port, ByteBuffer* packet) const;
};