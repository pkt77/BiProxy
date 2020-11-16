#include "data/Player.h"

#include <Proxy.h>

void Player::connect(Server* target) {
    proxy->getJeSocket()->createSocket(this, target);
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
    proxy->getJeSocket()->send(getSocket(), packet);
}