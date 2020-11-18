#include "data/Player.h"

#include <Proxy.h>

bool Player::connect(Server* target) {
    bool connected = proxy->getJeSocket()->createSocket(this, target);

    if (connected) {
        ByteBuffer* login = ByteBuffer::allocateBuffer(100);

        handshakePacket.write(login);
        proxy->getJeSocket()->send(connectingSocket, login);
        login->release();
        login->use();

        login->writeByte(0);
        login->writeString(username, true);
        login->prefixLength();

        proxy->getJeSocket()->send(connectingSocket, login);
        login->release();
    }

    return connected;
}

void Player::disconnect(std::string& reason) const {
    ByteBuffer* kick = ByteBuffer::allocateBuffer(100);

    kick->writeByte(0);
    kick->writeString("{\"text\":\"" + reason + "\", \"color\":\"red\"}", true);
    kick->prefixLength();

    sendPacket(kick);
    kick->release();
}

void Player::sendPacket(ByteBuffer* packet) const {
    proxy->getJeSocket()->send(socket, packet);
}