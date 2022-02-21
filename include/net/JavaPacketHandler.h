#pragma once

#include <net/protocol/Protocol.h>

class Proxy;

class JavaPacketHandler {
private:
    Proxy* proxy;

public:
    JavaPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(Connection* from) const;

    static bool packetConsumed(Connection* from);

    static void sendToDefaultServer(Connection* from);

    static void disconnect(Connection* from) ;
};