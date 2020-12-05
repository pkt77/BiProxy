#include "net/BedrockClientHandler.h"

#include "Proxy.h"
#include <iostream>

#define LOGIN 0x01

void BedrockClientHandler::handle(const void* socket, unsigned char* address, unsigned short port, ByteBuffer* packet) const {
    int length = packet->readVarInt();
    unsigned char id = packet->readUnsignedByte();

    switch (id) {
        case LOGIN: {
            int version = packet->readInt();

            std::cout << "Game " << version << ' ' << packet->readVarInt() << packet->readString<int>(&ByteBuffer::readIntLE) << ' '
                      << packet->readString<int>(&ByteBuffer::readIntLE) << std::endl;

            ByteBuffer* login = ByteBuffer::allocateBuffer(100);
            login->writeByte(0x02);
            login->writeInt(2);
            login->prefixLength();
            proxy->getRakNetPacketHandler().sendEncapsulated(socket, login, 0, true);
            break;
        }

        default:
            std::cout << "Unsupported Bedrock ID: " << +id << std::endl;
    }
}