#include "net/RakNetPacketHandler.h"

#include <packets/Packet.h>
#include "Proxy.h"

void RakNetPacketHandler::handle(const void* address, const char* payload, unsigned short size) const {
    Packet got(payload);

    got.setOffset(0);

    long long time = got.readLong();
    long long id = 13253860892328930865LL;

    Packet packet;

    packet.writeByte(0x1c);

    packet.writeLong(time);
    packet.writeLong(id);

    for (char byte: MAGIC) {
        packet.writeByte(byte);
    }

    packet.writeString(std::string("MCPE;ProxyServer;408;1.16.20;0;10;13253860892328930865;BedrockTest;Survival;1;19132;19133;"), false);

    proxy->getBeSocket()->send(&address, packet.getBuffer(), packet.getRealSize());
}