#include "net/RakNetPacketHandler.h"

#include <iostream>
#include <zlib.h>

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
#define NEW_INCOMING_CONNECTION 0x13
#define DISCONNECT 0x15
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

#define NACK 0xA0
#define ACK 0xC0

#define GAME1 0xFD
#define GAME2 0xFE

#define UNRELIABLE 0
#define UNRELIABLE_SEQUENCED 1
#define RELIABLE 2
#define RELIABLE_ORDERED 3
#define RELIABLE_SEQUENCED 4
#define UNRELIABLE_ACK 5
#define RELIABLE_ACK 6
#define RELIABLE_ORDERED_ACK 7

#define CHUNK 16384

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

std::map<short, ByteBuffer**> FRAGMENTS;

void RakNetPacketHandler::handle(const void* socket, unsigned char address[], unsigned short port, ByteBuffer* packet) const {
    unsigned char id = packet->readUnsignedByte();

/*    if (id != 1) {
        std::cout << packet->getSize() << " RakNet ID: " << +id << ", from " << +address[0] << '.' << +address[1] << '.' << +address[2] << '.' << +address[3] << ':'
                  << port << std::endl;
    }*/
    switch (id) {
        case CONNECTED_PING: {
            long long ping = packet->readLong();
            ByteBuffer* pong = ByteBuffer::allocateBuffer(10);

            pong->writeByte(CONNECTED_PONG);
            pong->writeLong(ping);
            pong->writeLong(currentTimeMillis());

            proxy->getBeSocket()->send(socket, pong);
            pong->release();
            break;
        }

        case UNCONNECTED_PING:
        case UNCONNECTED_PING_OPEN_CONNECTIONS: {
            if (packet->getSize() < 33) {
                break;
            }

            long long time = packet->readLong();
            ByteBuffer* pong = ByteBuffer::allocateBuffer(motd.length() + 35);

            pong->writeByte(UNCONNECTED_PONG);

            pong->writeLong(time);
            pong->writeLong(proxy->getId());
            pong->writeBytes(MAGIC, 16);
            pong->writeString<unsigned short>(motd, &ByteBuffer::writeUnsignedShort);

            proxy->getBeSocket()->send(socket, pong);
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
                inc->writeLong(proxy->getId());

                proxy->getBeSocket()->send(socket, inc);
                break;
            }

            unsigned short mtu = packet->readableBytes() + 46;
            ByteBuffer* reply = ByteBuffer::allocateBuffer(26);

            reply->writeByte(OPEN_CONNECTION_REPLY_1);
            reply->writeBytes(MAGIC, 16);
            reply->writeLong(proxy->getId());
            reply->writeBoolean(false);
            reply->writeShort(mtu);
            std::cout << "MTU: " << mtu << std::endl;
            proxy->getBeSocket()->send(socket, reply);
            reply->release();
            break;
        }

        case OPEN_CONNECTION_REQUEST_2: {
            packet->setOffset(17);

            short mtu;

            std::cout << "IPv" << +packet->readUnsignedByte() << ' ' << +ByteBuffer::flip(packet->readUnsignedByte()) << '.'
                      << +ByteBuffer::flip(packet->readUnsignedByte()) << '.'
                      << +ByteBuffer::flip(packet->readUnsignedByte()) << '.' << +ByteBuffer::flip(packet->readUnsignedByte()) << ':' << packet->readUnsignedShort()
                      << ' '
                      << (mtu = packet->readShort()) << ' ' << packet->readLong() << std::endl;

            ByteBuffer* reply = ByteBuffer::allocateBuffer(20);

            reply->writeByte(OPEN_CONNECTION_REPLY_2);
            reply->writeBytes(MAGIC, 16);
            reply->writeLong(proxy->getId());
            reply->writeByte(4);
            reply->writeUnsignedByte(ByteBuffer::flip(address[0]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[1]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[2]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[3]));
            reply->writeUnsignedShort(port);
            reply->writeShort(mtu);
            reply->writeBoolean(false);

            proxy->getBeSocket()->send(socket, reply);
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
            unsigned int sequence = packet->readInt24bit();

            while (packet->isReadable()) {
                char flags = packet->readByte();
                unsigned short bytes = (packet->readUnsignedShort() + 7) >> 3;
                unsigned char reliability = (flags & 0b11100000) >> 5; // first 3 bits equal the reliability id
                bool fragmented = (flags & 0b00010000) != 0; // 4th bit is 1 if fragmented

                //std::cout << sequence << ' ' << +reliability << ' ' << bytes << std::endl;

                if (isReliable(reliability)) {
                    unsigned int index = packet->readInt24bit();
                    ByteBuffer* ack = ByteBuffer::allocateBuffer(8);

                    ack->writeByte(ACK);
                    ack->writeShort(1);
                    ack->writeBoolean(true);
                    ack->writeInt24bit(sequence);

                    proxy->getBeSocket()->send(socket, ack);
                    ack->release();
                }

                if (isSequenced(reliability)) {
                    unsigned int index = packet->readInt24bit();
                }

                if (isOrdered(reliability)) {
                    unsigned int index = packet->readInt24bit();
                    char channel = packet->readByte();
                    //   std::cout << "Channel " << +packet->readByte() << std::endl;
                }

                if (fragmented) {
                    int fSize = packet->readInt();
                    short fId = packet->readShort();
                    int fIndex = packet->readInt();

                    //std::cout << "Fragment " << fSize << ' ' << fId << ' ' << fIndex << std::endl;

                    ByteBuffer* part = ByteBuffer::allocateBuffer(bytes);
                    ByteBuffer** parts = FRAGMENTS[fId];

                    if (parts == nullptr) {
                        FRAGMENTS[fId] = parts = new ByteBuffer* [fSize];
                    }

                    part->writeBytes(packet->getBuffer() + packet->getOffset(), bytes);
                    parts[fIndex] = part;

                    if (fSize == fIndex + 1) {
                        unsigned int size = 0;

                        for (unsigned int i = 0; i < fSize; i++) {
                            size += parts[i]->getSize();
                        }

                        ByteBuffer* fullPacket = ByteBuffer::allocateBuffer(size);
                        std::cout << "Fragment completed  " << size << std::endl;

                        for (unsigned int i = 0; i < fSize; i++) {
                            part = parts[i];

                            fullPacket->writeBytes(part->getBuffer(), part->getSize());
                            part->release();
                        }

                        FRAGMENTS.erase(fId);
                        delete[] parts;

                        fullPacket->setOffset(0);
                        handle(socket, address, port, fullPacket);
                    }
                }

                unsigned int offset = packet->getOffset();

                if (!fragmented) {
                    handle(socket, address, port, packet);
                }

                packet->setOffset(offset + bytes);
            }
            std::cout << std::endl;
            break;
        }

        case CONNECTION_REQUEST: {
            long long guid = packet->readLong();
            long long time = packet->readLong();
            bool security = packet->readBoolean();
            ByteBuffer* reply = ByteBuffer::allocateBuffer(100);

            reply->writeByte(CONNECTION_REQUEST_ACCEPTED);
            reply->writeUnsignedByte(4);
            reply->writeUnsignedByte(ByteBuffer::flip(address[0]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[1]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[2]));
            reply->writeUnsignedByte(ByteBuffer::flip(address[3]));
            reply->writeUnsignedShort(port);
            reply->writeShort(0); //Unknown
            reply->writeBytes(INTERNAL_ADDRESSES, sizeof(INTERNAL_ADDRESSES));
            reply->writeLong(time);
            reply->writeLong(currentTimeMillis());

            sendEncapsulated(socket, reply, 0x40, false);
            reply->release();

            break;
        }

        case NEW_INCOMING_CONNECTION: {
            std::cout << "Connection completed" << std::endl;
            break;
        }

        case DISCONNECT: {
            std::cout << "Disconnected" << std::endl;
            break;
        }

        case NACK:
        case ACK: {
            short count = packet->readShort();

            for (short i = 0; i < count; i++) {
                if (packet->readBoolean()) {
                    std::cout << "Got Ack " << packet->readInt24bit() << std::endl;
                } else {
                    std::cout << "Got Ack " << packet->readInt24bit() << ':' << packet->readInt24bit() << std::endl;
                }
            }
            break;
        }

        case GAME1:
        case GAME2: {
            z_stream stream{};
            int status = inflateInit2(&stream, -MAX_WBITS); //no header bytes
            unsigned char out[CHUNK] = {0};
            unsigned read;
            ByteBuffer* inflated = ByteBuffer::allocateBuffer(packet->readableBytes() * 8);

            stream.avail_in = packet->readableBytes();
            stream.next_in = (unsigned char*) (packet->getBuffer() + packet->getOffset());

            while (status == Z_OK) {
                stream.avail_out = CHUNK;
                stream.next_out = out;

                status = inflate(&stream, Z_NO_FLUSH);
                read = CHUNK - stream.avail_out;
                inflated->writeBytes(out, read);
            }
            inflated->setOffset(0);

            inflateEnd(&stream);

            bedrockClientHandler.handle(socket, address, port, inflated);

            inflated->release();
            packet->release();
            break;
        }

        default:
            std::cout << "Unsupported RakNet ID: " << +id << std::endl;
    }

    //packet->release(); TODO can't release sometimes when ecap
}

void RakNetPacketHandler::sendEncapsulated(const void* socket, ByteBuffer* packet, unsigned char reliability, bool deflate) const {
    ByteBuffer* encap = ByteBuffer::allocateBuffer(10 + packet->getSize());
    static volatile int SEQUENCE = 0;

    encap->writeUnsignedByte(FRAME_SET5);
    encap->writeInt24bit(SEQUENCE++);
    encap->writeUnsignedByte(reliability/* << 5*/);

    if (deflate) {
        encap->setOffset(7); // Skip size until we deflate
        encap->writeUnsignedByte(GAME2);

        z_stream stream{};
        int status = deflateInit2(&stream, 7, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY); //no header bytes
        unsigned char out[CHUNK] = {0};
        unsigned read;

        stream.avail_in = packet->getSize();
        stream.next_in = (unsigned char*) packet->getBuffer();

        while (status == Z_OK) {
            stream.avail_out = CHUNK;
            stream.next_out = out;

            status = ::deflate(&stream, Z_FINISH);
            read = CHUNK - stream.avail_out;
            encap->writeBytes(out, read);
        }

        encap->setOffset(5); // Go back and write size
        encap->writeUnsignedShort((encap->getSize() - 5) << 3);
        //encap->writeInt24bit(0);
    } else {
        encap->writeUnsignedShort(packet->getSize() << 3);
        if (reliability != 0) {
            encap->writeInt24bit(0);
        }
        encap->writeBytes(packet->getBuffer(), packet->getSize());
    }

    proxy->getBeSocket()->send(socket, encap);
    encap->release();
}

void RakNetPacketHandler::setMotd(const std::string& line1, const std::string& line2) {
    motd = std::string("MCPE;" + line1 + ";465;1.17.34;0;10;" + std::to_string(proxy->getId()) + ';' + line2 + ";Survival;1;19132;19133;");
}