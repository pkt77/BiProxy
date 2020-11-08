#include "net/RakNetPacketHandler.h"

#include <iostream>

#include <packets/Packet.h>
#include "Proxy.h"

#define CONNECTED_PING 0x00
#define UNCONNECTED_PING 0x01
#define UNCONNECTED_PING_OPEN_CONNECTIONS 0x02
#define CONNECTED_PONG 0x03
#define OPEN_CONNECTION_REQUEST_1 0x05
#define OPEN_CONNECTION_REPLY_1 0x06
#define OPEN_CONNECTION_REQUEST_2 0x07
#define OPEN_CONNECTION_REPLY_2 0x08
#define CONNECTION_REQUEST 0x09
#define CONNECTION_REQUEST_ACCEPTED 0x10
#define UNCONNECTED_PONG 0x1c

void RakNetPacketHandler::handle(const void* address, const char* payload, unsigned short size) const {
    //std::cout << size << " ID: " << +payload[0] << std::endl;
    switch (payload[0]) {
        case UNCONNECTED_PING:
        case UNCONNECTED_PING_OPEN_CONNECTIONS: {
            if (size < 33) {
                break;
            }

            ByteBuffer got(payload, size);

            got.setOffset(1);

            long long time = got.readLong();
            long long id = 13253860892328930865LL;

            ByteBuffer pong;

            pong.writeByte(UNCONNECTED_PONG);

            pong.writeLong(time);
            pong.writeLong(id);
            pong.writeBytes(MAGIC, 16);

            pong.writeString("MCPE;ProxyServer;408;1.16.20;0;10;13253860892328930865;BedrockTest;Survival;1;19132;19133;", false);
            proxy->getBeSocket()->send(address, pong);
            break;
        }

        default:
            std::cout << "Unknown RakNet ID: " << +payload[0] << std::endl;
    }
}