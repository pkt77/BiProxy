#include "data/Player.h"

#include <Proxy.h>
#include <utils/Encryption.h>

bool Player::connect(Server* target) {
    bool connected = proxy->getJeSocket()->createSocket(this, target);

    if (connected) {
        ByteBuffer* login = ByteBuffer::allocateBuffer(100);

        handshakePacket.write(login);
        proxy->getJeSocket()->send(connectingSocket, login);
        login->release();
        login->use();

        login->writeByte(0);
        login->writeString<int>(username, &ByteBuffer::writeVarInt);
        login->prefixLength();

        proxy->getJeSocket()->send(connectingSocket, login);
        login->release();
    }

    return connected;
}

void Player::disconnect(const std::string& reason) const {
    ByteBuffer* kick = ByteBuffer::allocateBuffer(300);

    kick->writeByte(0);
    kick->writeString<int>("{\"text\":\"" + reason + "\", \"color\":\"red\"}", &ByteBuffer::writeVarInt);
    kick->prefixLength();

    sendPacket(kick);
    kick->release();
}

void Player::sendPacket(ByteBuffer* packet) const {
    if (!secret.empty()) {
        Encryption::encode(secret, packet);
    }
    proxy->getJeSocket()->send(socket, packet);
}