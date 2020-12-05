#include "net/JavaServerPacketHandler.h"
#include <iostream>
#include "Proxy.h"

void JavaServerPacketHandler::handle(Connection* from, ByteBuffer* packet) const {
    int length;
    unsigned int offset;
    unsigned char id;
    unsigned char bytes;

    while (packet->isReadable()) {
        length = packet->readVarInt();
        offset = packet->getOffset();
        id = packet->readByte();
        bytes = varIntLength(length);

        //std::cout << length << ' ' << packet->readableBytes() << " SER ID: " << +id << std::endl;

        switch (id) {
            /*     case 0:
                     std::cout << packet->readString() << std::endl;
                     break;*/

            case 0x02:
                //std::cout << "Got Login" << std::endl;
                break;

            case 0x24: {
                // std::ofstream out("data.nbt", std::ios::binary);

                //out.write(packet->getBuffer() + packet->getOffset(), length - 1);
                break;
            }

            case 0x17: {
                //std::cout << packet->readString(&ByteBuffer::readVarInt) << std::endl;
                break;
            }

            case 0x19:
                //std::cout << packet->readString(&ByteBuffer::readVarInt) << std::endl;
                break;

            default:
                break;
        }

        if (proxy->getJeSocket()->send(from->owner->getSocket(), packet->getBuffer() + offset - bytes, bytes + length)) {
            packet->setOffset(offset + length);
        } else {
            packet->setOffset(packet->getSize());
        }
    }

    packet->release();
}

void JavaServerPacketHandler::disconnect(Connection* from) const {
    // std::string msg = "Server disconnected";
    // from->owner->disconnect(msg);
}