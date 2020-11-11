#include "net/RakNetPacketHandler.h"

#include <iostream>
#include "Proxy.h"

#define RAKNET_PROTOCOL_VERSION 10

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
#define INCOMPATIBLE_PROTOCOL_VERSION 0x19

#define FRAME_SET1 0x80
#define FRAME_SET2 0x81
#define FRAME_SET3 0x82
#define FRAME_SET4 0x83
#define FRAME_SET5 0x84
#define FRAME_SET6 0x85
#define FRAME_SET7 0x86
#define FRAME_SET8 0x87
#define FRAME_SET9 0x88
#define FRAME_SET10 0x89
#define FRAME_SET11 0x8A
#define FRAME_SET12 0x8B
#define FRAME_SET13 0x8C
#define FRAME_SET14 0x8D

#define UNRELIABLE 0
#define UNRELIABLE_SEQUENCED 1
#define RELIABLE 2
#define RELIABLE_ORDERED 3
#define RELIABLE_SEQUENCED 4
#define UNRELIABLE_ACK 5
#define RELIABLE_ACK 6
#define RELIABLE_ORDERED_ACK 7

bool isReliable(unsigned char id) {
    switch (id) {
        case RELIABLE:
        case RELIABLE_ORDERED:
        case RELIABLE_SEQUENCED:
        case RELIABLE_ACK:
        case RELIABLE_ORDERED_ACK:
            return true;
        default:
            return false;
    }
}

bool isOrdered(unsigned char id) {
    switch (id) {
        case UNRELIABLE_SEQUENCED:
        case RELIABLE_ORDERED:
        case RELIABLE_SEQUENCED:
        case RELIABLE_ORDERED_ACK:
            return true;
        default:
            return false;
    }
}

bool isSequenced(unsigned char id) {
    return id == UNRELIABLE_SEQUENCED || id == RELIABLE_SEQUENCED;
}

void RakNetPacketHandler::handle(const void* address, ByteBuffer* packet) const {
    unsigned char id = packet->readUnsignedByte();

    //std::cout << size << " ID: " << +payload[0] << std::endl;
    switch (id) {
        case CONNECTED_PING: {
            long long ping = packet->readLong();
            ByteBuffer* pong = ByteBuffer::allocateBuffer(10);

            pong->writeByte(CONNECTED_PONG);
            pong->writeLong(ping);
            pong->writeLong(currentTimeMillis());

            proxy->getBeSocket()->send(address, pong);
            pong->release();
            break;
        }

        case UNCONNECTED_PING:
        case UNCONNECTED_PING_OPEN_CONNECTIONS: {
            if (packet->getSize() < 33) {
                break;
            }

            std::string motd("MCPE;ProxyServer;408;1.16.20;0;10;");
            motd += std::to_string(GUID);
            motd += ";BedrockTest;Survival;1;19132;19133;";

            long long time = packet->readLong();
            ByteBuffer* pong = ByteBuffer::allocateBuffer(motd.length() + 35);

            pong->writeByte(UNCONNECTED_PONG);

            pong->writeLong(time);
            pong->writeLong(GUID);
            pong->writeBytes(MAGIC, 16);
            pong->writeString(motd, false);

            proxy->getBeSocket()->send(address, pong);
            pong->release();
            break;
        }

        case OPEN_CONNECTION_REQUEST_1: {
            packet->setOffset(17);

            if (packet->readByte() != RAKNET_PROTOCOL_VERSION) {
                ByteBuffer* inc = ByteBuffer::allocateBuffer(25); //TODO cache

                inc->writeByte(INCOMPATIBLE_PROTOCOL_VERSION);
                inc->writeByte(RAKNET_PROTOCOL_VERSION);
                inc->writeBytes(MAGIC, 16);
                inc->writeLong(GUID);

                proxy->getBeSocket()->send(address, inc);
                break;
            }

            unsigned short mtu = packet->readableBytes() + 46;
            ByteBuffer* reply = ByteBuffer::allocateBuffer(26);

            reply->writeByte(OPEN_CONNECTION_REPLY_1);
            reply->writeBytes(MAGIC, 16);
            reply->writeLong(GUID);
            reply->writeBoolean(false);
            reply->writeShort(mtu);
            std::cout << "MTU: " << mtu << std::endl;
            proxy->getBeSocket()->send(address, reply);
            reply->release();
            break;
        }

        case OPEN_CONNECTION_REQUEST_2: {
            packet->setOffset(17);

            short mtu;
/*
            for (int i = packet.getOffset(); i < packet.getOffset() + 7; i++) {
                std::cout << std::hex << +packet.getBuffer()[i] << ',';
            }*/

            std::cout << "IPv" << +packet->readUnsignedByte() << ' ' << +ByteBuffer::flip(packet->readUnsignedByte()) << '.'
                      << +ByteBuffer::flip(packet->readUnsignedByte()) << '.'
                      << +ByteBuffer::flip(packet->readUnsignedByte()) << '.' << +ByteBuffer::flip(packet->readUnsignedByte()) << ':' << packet->readUnsignedShort()
                      << ' '
                      << (mtu = packet->readShort()) << ' ' << packet->readLong() << std::endl;

            ByteBuffer* reply = ByteBuffer::allocateBuffer(20);

            reply->writeByte(OPEN_CONNECTION_REPLY_2);
            reply->writeBytes(MAGIC, 16);
            reply->writeLong(GUID);
            reply->writeByte(4);
            reply->writeUnsignedByte(ByteBuffer::flip(192));
            reply->writeUnsignedByte(ByteBuffer::flip(168));
            reply->writeUnsignedByte(ByteBuffer::flip(1));
            reply->writeUnsignedByte(ByteBuffer::flip(66));
            reply->writeUnsignedShort(19132);
            reply->writeShort(mtu);
            reply->writeBoolean(false);

            proxy->getBeSocket()->send(address, reply);
            reply->release();
            break;
        }

        case FRAME_SET1:
        case FRAME_SET2:
        case FRAME_SET3:
        case FRAME_SET4:
        case FRAME_SET5:
        case FRAME_SET6:
        case FRAME_SET7:
        case FRAME_SET8:
        case FRAME_SET9:
        case FRAME_SET10:
        case FRAME_SET11:
        case FRAME_SET12:
        case FRAME_SET13:
        case FRAME_SET14: {
            while (packet->isReadable()) {
                unsigned int sequence = packet->readInt24bit();

                char flags = packet->readByte();
                unsigned short bits = packet->readUnsignedShort();
                unsigned char reliability = (flags & 0b11100000) >> 5;
                unsigned int index = packet->readInt24bit();

                //std::cout << +reliability << ' ' << bits << ' ' << index << std::endl;

                if (isOrdered(reliability)) {
                    std::cout << "Channel " << +packet->readByte() << std::endl;
                }

                if ((flags & 0b00010000) != 0) { // 4th bit is 1 if fragmented
                    int fSize = packet->readInt();
                    short fId = packet->readShort();
                    int fIndex = packet->readInt();

                    std::cout << "Fragment " << fSize << ' ' << fId << ' ' << fIndex << std::endl;
                }

                handle(address, packet);
                packet->setOffset(packet->getOffset() + (bits / 8));
            }
            break;
        }

        case CONNECTION_REQUEST: {
            long long guid = packet->readLong();
            long long time = packet->readLong();
            ByteBuffer* reply = ByteBuffer::allocateBuffer(100);
            ByteBuffer* encap = ByteBuffer::allocateBuffer(100);

            reply->writeByte(CONNECTION_REQUEST_ACCEPTED);
            reply->writeUnsignedByte(4);
            reply->writeUnsignedByte(ByteBuffer::flip(192));
            reply->writeUnsignedByte(ByteBuffer::flip(168));
            reply->writeUnsignedByte(ByteBuffer::flip(1));
            reply->writeUnsignedByte(ByteBuffer::flip(66));
            reply->writeUnsignedShort(19132);
            reply->writeShort(0); //Unknown
            reply->writeBytes(ADDRESSES, 70);
            reply->writeLong(time);
            reply->writeLong(currentTimeMillis());

            encap->writeByte(FRAME_SET1);
            encap->writeInt24bit(0);
            encap->writeByte(0);
            encap->writeUnsignedShort(reply->getSize() * 8);
            encap->writeBytes(reply->getBuffer(), reply->getSize());

            proxy->getBeSocket()->send(address, encap);
            reply->release();
            encap->release();
            break;
        }

        default:
            std::cout << "Unsupported RakNet ID: " << +id << std::endl;
    }

    packet->release();
}