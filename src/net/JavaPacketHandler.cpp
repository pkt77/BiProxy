#include <net/JavaPacketHandler.h>

#include "Proxy.h"

#include <iostream>
#include <packets/HandshakePacket.h>
#include <packets/CompressionPacket.h>
#include <set>

std::set<const void*> pinging; // insert() doesn't work if this is declared in header???

void JavaPacketHandler::handle(const void* address, const char payload[], unsigned short size) const {
    Packet packet(payload);
    std::cout << "ID: " << +packet.getId() << std::endl;

    switch (packet.getId()) {
        case 0: {
            if (pinging.find(address) == pinging.end()) {
                HandshakePacket hand(payload);

                std::cout << hand.getVersion() << std::endl;
                std::cout << hand.getAddress() << std::endl;
                std::cout << hand.getPort() << std::endl;
                std::cout << +hand.getState() << std::endl;

                pinging.insert(address);
                return;
            }

            Packet motd;

            motd.writeByte(0);
            motd.writeString(proxy->getMotdString(), true);
            motd.setOffset(0);

            Packet fin;

            fin.writeVarInt(motd.getRealSize());

            for (unsigned int i = 0; i < motd.getRealSize(); i++) {
                fin.writeByte(motd.getBuffer()[i]);
            }

            proxy->getJeSocket()->send(address, fin);
            break;
        }

        case 1: {
            proxy->getJeSocket()->send(address, payload, size);
            break;
        }

        case 3: {
            CompressionPacket comp(payload);

            std::cout << comp.getThreshold() << std::endl;
            break;
        }

        default:
            std::cout << "UNSUPPORTED PACKET " << +packet.getId() << std::endl;
    }
}

void JavaPacketHandler::disconnect(const void* address) const {
    pinging.erase(address);
}
