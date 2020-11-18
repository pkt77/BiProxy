#pragma once

#include <packets/HandshakePacket.h>
#include "Server.h"

class Proxy;

class Player {
private:
    void* socket;
    std::string username;

protected:
    Proxy* proxy;

public:
    HandshakePacket handshakePacket;

    void* serverSocket = nullptr;
    void* connectingSocket = nullptr;

    Player(Proxy* proxy, void* socket) : proxy(proxy), socket(socket) {}

    bool connect(Server* target);

    void disconnect(std::string& reason) const;

    void sendPacket(ByteBuffer* packet) const;

    void* getSocket() const {
        return socket;
    }

    const std::string& getUsername() const {
        return username;
    }

    void setUsername(const std::string& username) {
        this->username = username;
    }
};

struct JUUID {
    long long mostSigBits;
    long long leastSigBits;
};

struct Connection {
    void* socket;
    bool shook;
    Player* owner;
};