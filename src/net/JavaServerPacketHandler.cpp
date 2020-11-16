#include "net/JavaServerPacketHandler.h"
#include <iostream>

void JavaServerPacketHandler::handle(Connection* from, ByteBuffer* packet) const {
    int length = packet->readVarInt();
    unsigned int offset = packet->getOffset();
    unsigned char id = packet->readByte();

    std::cout << length << ' ' << packet->readableBytes() << " ID: " << +id << std::endl;

    switch (id) {
        case 0:
            std::cout << packet->readString() << std::endl;
            break;

        case 0x19:
            std::cout << packet->readString() << std::endl;
            break;
    }
}

void JavaServerPacketHandler::disconnect(Connection* from) const {
    std::string msg = "Server disconnected";
    from->owner->disconnect(msg);
}