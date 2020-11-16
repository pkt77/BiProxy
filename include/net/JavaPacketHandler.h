#pragma once

#include <utils/ByteBuffer.h>
#include <data/Player.h>

class Proxy;

class JavaPacketHandler {
private:
    Proxy* proxy;

public:
    JavaPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(Connection* from, ByteBuffer* packet) const;

    void disconnect(Connection* from) const;
};