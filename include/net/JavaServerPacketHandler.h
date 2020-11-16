#pragma once

#include <utils/ByteBuffer.h>
#include <data/Player.h>

class Proxy;

class JavaServerPacketHandler {
private:
    Proxy* proxy;

public:
    JavaServerPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(Connection* from, ByteBuffer* packet) const;

    void disconnect(Connection* from) const;
};