#include "net/protocol/ClientStatus.h"

#include "Proxy.h"

bool ClientStatus::request(Connection* from) {
    std::string motdJson = Proxy::instance()->getDefaultPing().toJSON();
    ByteBuffer* motd = ByteBuffer::allocateBuffer(motdJson.length() + 5);

    motd->writeByte(STATUS);
    motd->writeString<int>(motdJson, &ByteBuffer::writeVarInt);
    motd->prefixLength();

    Proxy::instance()->getJeSocket()->send(from->socket, motd);
    motd->release();
    return true;
}

bool ClientStatus::ping(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    ByteBuffer* pong = ByteBuffer::allocateBuffer(10);
    long long time = buffer->readLong();

    pong->writeVarInt(9);
    pong->writeByte(PING);
    pong->writeLong(time);

    if (from->owner == nullptr) {
        Proxy::instance()->getJeSocket()->send(from->socket, pong);
    } else {
        Proxy::instance()->getJeSocket()->send(from->owner->connectingSocket, pong);
    }

    pong->release();
    return true;
}