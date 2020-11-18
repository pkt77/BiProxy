#include <net/JavaPacketHandler.h>

#include "Proxy.h"

#include <iostream>
#include <packets/HandshakePacket.h>
#include <packets/CompressionPacket.h>
#include <set>

#define HANDSHAKE 0
#define STATUS 0
#define PING 1

void JavaPacketHandler::handle(Connection* from, ByteBuffer* packet) const {
    int length;
    unsigned int offset;
    unsigned char id;
    unsigned char bytes;

    while (packet->isReadable()) {
        length = packet->readVarInt();
        offset = packet->getOffset();
        id = packet->readByte();
        bytes = varIntLength(length);

        // std::cout << length << ' ' << packet->readableBytes() << " CLI ID: " << +id << std::endl;

        switch (id) {
            case HANDSHAKE: {
                if (from->owner != nullptr && from->owner->connectingSocket != nullptr) {
                    break;
                }
                if (from->shook) {
                    if (from->owner == nullptr) {
                        ByteBuffer* motd = ByteBuffer::allocateBuffer(proxy->getMotdString().length() + 5);

                        motd->writeByte(STATUS);
                        motd->writeString(proxy->getMotdString(), true);
                        motd->prefixLength();

                        proxy->getJeSocket()->send(from->socket, motd);
                        motd->release();
                    } else {
                        from->owner->setUsername(packet->readString());
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

                hand.read(packet);

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
                long long time = packet->readLong();

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

            comp.read(packet);

            std::cout << comp.getThreshold() << std::endl;
            break;
        }*/

            default:
                break;
        }

        if (from->owner != nullptr && from->owner->connectingSocket != nullptr) {
            proxy->getJeSocket()->send(from->owner->connectingSocket, packet->getBuffer() + offset - bytes, bytes + length);
        }

        packet->setOffset(offset + length);
    }

    packet->release();
}

void JavaPacketHandler::disconnect(Connection* from) const {
}