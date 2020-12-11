#include <net/JavaPacketHandler.h>

#include "Proxy.h"

#include <iostream>

#define HANDSHAKE 0
#define STATUS 0
#define PING 1

void JavaPacketHandler::handle(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    unsigned int startOffset;
    int length;
    unsigned int offset;
    unsigned char bytes;

    bool full = true;
    bool fromClient = from->owner == nullptr || from->socket == from->owner->getSocket();

    while (buffer->isReadable()) {
        startOffset = buffer->getOffset();

        try {
            length = buffer->readVarInt();
        } catch (char* e) {
            full = false;
            std::cout << "Header was incomplete " << e << std::endl;
        }

        offset = buffer->getOffset();

        if (!full || length > buffer->readableBytes()) {
            memcpy(buffer->getBuffer(), buffer->getBuffer() + startOffset, buffer->getSize() - startOffset);
            buffer->setSize(buffer->getSize() - startOffset);
            buffer->setOffset(0);

            if (full) {
                buffer->ensureWritable(length);
            }

            from->processing = false;
            return;
        }

        bytes = offset - startOffset;

        if (fromClient) {
            if (!handleClient(from) && from->owner != nullptr && from->owner->connectingSocket != nullptr) {
                if (!proxy->getJeSocket()->send(from->owner->connectingSocket, buffer->getBuffer() + startOffset, bytes + length)) {
                    break;
                }
            }
        } else {
            if (!handleServer(from)) {
                if (proxy->getJeSocket()->send(from->owner->getSocket(), buffer->getBuffer() + startOffset, bytes + length)) {
                    if (length > 8000) {
                        // TODO Find out why big packets like chunks need to wait, as if the buffer needs to flush... Otherwise they end up corrupted on the fromClient
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1500));
                    }
                } else {
                    break;
                }
            }
        }

        buffer->setOffset(offset + length);
    }

    buffer->setSize(0);
    buffer->setOffset(0);
    from->processing = false;
}

bool JavaPacketHandler::handleClient(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    switch (buffer->readUnsignedByte()) {
        case HANDSHAKE: {
            if (from->owner != nullptr && from->owner->connectingSocket != nullptr) { //TODO replace with play state
                return false;
            }

            if (from->shook) {
                if (from->owner == nullptr) {
                    std::string motdJson = proxy->getDefaultPing().toJSON();
                    ByteBuffer* motd = ByteBuffer::allocateBuffer(motdJson.length() + 5);

                    motd->writeByte(STATUS);
                    motd->writeString<int>(motdJson, &ByteBuffer::writeVarInt);
                    motd->prefixLength();

                    proxy->getJeSocket()->send(from->socket, motd);
                    motd->release();
                } else {
                    from->owner->setUsername(buffer->readString<int>(&ByteBuffer::readVarInt));
                    std::cout << from->owner->getUsername() << " connecting..." << std::endl;
                    std::string server = "default";
                    Server* hub = proxy->getServer(server);

                    if (hub == nullptr) {
                        std::string msg = "No default server";
                        from->owner->disconnect(msg);
                    } else if (!from->owner->connect(hub)) {
                        std::string msg = "Failed to connect to default server";
                        from->owner->disconnect(msg);
                    }
                }
                break;
            }

            HandshakePacket hand;

            hand.read(buffer);

            std::cout << hand.getVersion() << '-' << hand.getAddress() << ':' << hand.getPort() << '+' << +hand.getState() << std::endl;

            if (hand.getState() == 2) {
                from->owner = new Player(proxy, from->socket);
                from->owner->handshakePacket = hand;
            }
            from->shook = true;
            break;
        }

        case PING: {
            ByteBuffer* pong = ByteBuffer::allocateBuffer(10);
            long long time = buffer->readLong();

            pong->writeVarInt(9);
            pong->writeByte(PING);
            pong->writeLong(time);

            if (from->owner == nullptr) {
                proxy->getJeSocket()->send(from->socket, pong);
            } else {
                proxy->getJeSocket()->send(from->owner->connectingSocket, pong);
            }

            pong->release();
            break;
        }

/*        case 3: {
            CompressionPacket comp;

            comp.read(buffer);

            std::cout << comp.getThreshold() << std::endl;
            break;
        }*/

        default:
            return false;
    }
    return true;
}

bool JavaPacketHandler::handleServer(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    switch (buffer->readUnsignedByte()) {
        /*     case 0:
                 std::cout << buffer->readString() << std::endl;
                 break;*/

        case 0x02:
            //std::cout << "Got Login" << std::endl;
            break;

        case 0x24: {
            // std::ofstream out("data.nbt", std::ios::binary);

            //out.write(buffer->getBuffer() + buffer->getOffset(), length - 1);
            break;
        }

        case 0x17: {
            std::cout << buffer->readString<int>(&ByteBuffer::readVarInt) << std::endl;
            break;
        }

        case 0x19:
            std::cout << buffer->readString<int>(&ByteBuffer::readVarInt) << std::endl;
            break;

        default:
            return false;
    }
    return false; // Temporary
}

void JavaPacketHandler::disconnect(Connection* from) const {
    if (from->owner == nullptr || from->socket == from->owner->getSocket()) {
        if (from->owner != nullptr) {
            std::cout << from->owner->getUsername() << " Disconnected" << std::endl;
        }
    } else {
        std::string msg = "Server disconnected";
        // from->owner->disconnect(msg);
        std::cout << msg << std::endl;
    }
}