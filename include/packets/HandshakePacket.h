#pragma once

#include <iostream>
#include "Packet.h"

class HandshakePacket : public Packet {
private:
    static constexpr char FML[] = {0, 'F', 'M', 'L', 0};

    int version;
    std::string address;
    unsigned short port;
    bool isForge;
    unsigned char state;

public:
    void read(ByteBuffer* buffer) override {
        version = buffer->readVarInt();
        address = buffer->readString<int>(&ByteBuffer::readVarInt);
        port = buffer->readUnsignedShort();
        state = buffer->readVarInt();
    }

    void write(ByteBuffer* buffer) override {
        buffer->writeByte(0);
        buffer->writeVarInt(version);
        buffer->writeString<int>(address, &ByteBuffer::writeVarInt);
        buffer->writeUnsignedShort(port);
        buffer->writeUnsignedByte(state);
        buffer->prefixLength();
    }

    int getVersion() const {
        return version;
    }

    std::string& getAddress() {
        return address;
    }

    bool forge() const {
        return isForge;
    }

    unsigned short getPort() const {
        return port;
    }

    unsigned char getState() const {
        return state;
    }
};