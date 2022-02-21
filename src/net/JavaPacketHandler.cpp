#include <net/JavaPacketHandler.h>

#include "Proxy.h"
#include "net/protocol/ClientLogin.h"
#include <utils/Base64.h>

#include <iostream>
#include <utils/Encryption.h>

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
            if (!packetConsumed(from) && from->owner != nullptr && from->owner->connectingSocket != nullptr) {
                if (!proxy->getJeSocket()->send(from->owner->connectingSocket, buffer->getBuffer() + startOffset, bytes + length)) {
                    break;
                }
            }
        } else {
            if (!packetConsumed(from)) {
                if (!from->owner->getSecret().empty()) {
                    //Encryption::encode(from->owner->getSecret(), buffer->getBuffer() + startOffset, bytes + length);
                }
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

bool JavaPacketHandler::packetConsumed(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    unsigned char id = buffer->readUnsignedByte();
    PacketHandler handler = from->protocol->getHandler(id);

    return handler != nullptr && handler(from);
}

void JavaPacketHandler::sendToDefaultServer(Connection* from) {
    std::string server = "hub";
    Server* hub = Proxy::instance()->getServer(server);

    if (hub == nullptr) {
        std::string msg = "No default server";
        from->owner->disconnect(msg);
    } else if (!from->owner->connect(hub)) {
        std::string msg = "Failed to connect to default server";
        from->owner->disconnect(msg);
    }
}

void JavaPacketHandler::disconnect(Connection* from) {
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