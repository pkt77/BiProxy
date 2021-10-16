#pragma once

#include <packets/HandshakePacket.h>
#include "Server.h"

class Proxy;

class Player {
private:
    void* socket;

    std::string username;
    std::string uuid;

    std::string secret;
    int token;

protected:
    Proxy* proxy;

public:
    HandshakePacket handshakePacket;

    void* serverSocket = nullptr;
    void* connectingSocket = nullptr;

    Player(Proxy* proxy, void* socket) : proxy(proxy), socket(socket) {}

    bool connect(Server* target);

    void disconnect(const std::string& reason) const;

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

    const std::string& getUUID() const {
        return uuid;
    }

    void setUUID(const std::string& uuid) {
        Player::uuid = uuid;
    }


    const std::string& getSecret() const {
        return secret;
    }

    void setSecret(const std::string& secret) {
        Player::secret = secret;
    }

    int getToken() const {
        return token;
    }

    void setToken(int token) {
        Player::token = token;
    }
};

struct JUUID {
    long long mostSigBits;
    long long leastSigBits;
};

/*enum ConnectionState {
    HANDSHAKE,
    STATUS,

    LOGIN
};*/

struct Connection {
    void* socket;
    ByteBuffer* buffer;
    bool processing;
    unsigned char state;
    Player* owner;
};