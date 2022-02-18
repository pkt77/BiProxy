#pragma once

#include <data/Player.h>

class Proxy;

class JavaPacketHandler {
private:
    Proxy* proxy;

public:
    JavaPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(Connection* from) const;

    bool handleClient(Connection* from) const;

    bool handleServer(Connection* from) const;

    void sendToDefaultServer(Connection* from) const;

    void disconnect(Connection* from) const;
};