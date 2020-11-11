#include <net/JavaPacketHandler.h>

#include "Proxy.h"

#include <iostream>
#include <packets/HandshakePacket.h>
#include <packets/CompressionPacket.h>
#include <set>

#define HANDSHAKE 0
#define STATUS 0
#define PING 1

std::set<const void*> pinging; // insert() doesn't work if this is declared in header???
std::set<const void*> joining;

void JavaPacketHandler::handle(const void* address, ByteBuffer* packet) const {
    int length = packet->readVarInt();
    unsigned int offset = packet->getOffset();
    unsigned char id = packet->readByte();

    //std::cout << length << ' ' << packet->readableBytes() << " ID: " << +id << std::endl;

    switch (id) {
        case HANDSHAKE: {
            if (joining.find(address) != joining.end()) {
                std::cout << packet->readString() << std::endl;

                ByteBuffer* encr = ByteBuffer::allocateBuffer(100);

                encr->writeByte(0);
                encr->writeString(R"({"text":"Banned", "color":"red"})", true);
                encr->prefixLength();

                proxy->getJeSocket()->send(address, encr);
                encr->release();
                break;
            }

            if (pinging.find(address) != pinging.end()) {
                ByteBuffer* motd = ByteBuffer::allocateBuffer(proxy->getMotdString().length() + 5);

                motd->writeByte(STATUS);
                motd->writeString(proxy->getMotdString(), true);
                motd->prefixLength();

                proxy->getJeSocket()->send(address, motd);
                motd->release();
                break;
            }

            HandshakePacket hand;

            hand.read(packet);

            std::cout << hand.getVersion() << '-' << hand.getAddress() << ':' << hand.getPort() << '+' << +hand.getState() << std::endl;

            if (hand.getState() == 1) {
                pinging.insert(address);
            } else {
                joining.insert(address);
            }
            break;
        }

        case PING: {
            ByteBuffer* pong = ByteBuffer::allocateBuffer(10);
            long long time = packet->readLong();

            pong->writeVarInt(9);
            pong->writeByte(PING);
            pong->writeLong(time);

            proxy->getJeSocket()->send(address, pong);
            pong->release();
            break;
        }

        case 3: {
            CompressionPacket comp;

            comp.read(packet);

            std::cout << comp.getThreshold() << std::endl;
            break;
        }

        default:
            std::cout << "UNSUPPORTED PACKET " << +id << std::endl;
    }

    packet->setOffset(offset + length);

    if (packet->isReadable()) {
        handle(address, packet);
    } else {
        packet->release();
    }
}

void JavaPacketHandler::disconnect(const void* address) const {
    pinging.erase(address);
    joining.erase(address);
}