#include <net/JavaPacketHandler.h>

#include "Proxy.h"

#include <iostream>
#include <packets/HandshakePacket.h>
#include <packets/CompressionPacket.h>
#include <set>

std::set<const void*> pinging; // insert() doesn't work if this is declared in header???

void JavaPacketHandler::handle(const void* address, const char payload[], unsigned short size) const {
    ByteBuffer packet(payload, size);
    int length = packet.readVarInt();
    unsigned char id = packet.readByte();

    //std::cout << length << ' ' << size << " ID: " << +id << std::endl;

    switch (id) {
        case 0: {
            if (pinging.find(address) == pinging.end()) {
                HandshakePacket hand;

                hand.read(packet);

                std::cout << hand.getVersion() << '-' << hand.getAddress() << ':' << hand.getPort() << '+' << +hand.getState() << std::endl;

                pinging.insert(address);
                break;
            }

            ByteBuffer motd;

            motd.writeByte(0);
            motd.writeString(proxy->getMotdString(), true);
            motd.prefixLength();

            proxy->getJeSocket()->send(address, motd);
            break;
        }

        case 1: {
            packet.setOffset(packet.getSize());
            proxy->getJeSocket()->send(address, payload, size);
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

    if (packet.isReadable()) {
        handle(address, payload + packet.getOffset(), size - packet.getOffset());
    }
}

void JavaPacketHandler::disconnect(const void* address) const {
    pinging.erase(address);
}