#pragma once

#include <packets/HandshakePacket.h>
#include "Server.h"

class Proxy;

class Player {
private:
    void* socket;

protected:
    Proxy* proxy;

public:
    HandshakePacket handshakePacket;

    void* serverSocket = nullptr;
    void* connectingSocket = nullptr;

    Player(Proxy* proxy, void* socket) : proxy(proxy), socket(socket) {}

    void connect(Server* target);

    void disconnect(std::string& reason) const;

    void sendPacket(ByteBuffer* packet) const;

    void* getSocket() const {
        return socket;
    }
};

struct Connection {
    void* socket;
    bool shook;
    Player* owner;
};