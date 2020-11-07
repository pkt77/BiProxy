#pragma once

#include <packets/Packet.h>

class Proxy;

class JavaPacketHandler {
private:
    Proxy* proxy;

public:
    JavaPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(const void* address, const char payload[], unsigned short size) const;

    void disconnect(const void* address) const;
};