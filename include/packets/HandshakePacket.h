#pragma once

#include "Packet.h"

class HandshakePacket : public Packet {
private:
    static constexpr char FML[] = {0, 'F', 'M', 'L', 0};

    int version;
    char* address;
    unsigned short port;
    bool isForge;
    unsigned char state;

public:
    void read(ByteBuffer* buffer) override {
        version = buffer->readVarInt();
        address = buffer->readString();
        port = buffer->readUnsignedShort();
        state = buffer->readVarInt();
    }

    void write(ByteBuffer* buffer) override {
        buffer->writeByte(0);
        buffer->writeVarInt(version);
        auto& c = "localhost\00localhost\00""1ec83775-a408-47de-adef-1d537424f09e";

        buffer->writeString(std::string(std::begin(c), std::end(c)), true);
        buffer->writeUnsignedShort(port);
        buffer->writeUnsignedByte(state);
        buffer->prefixLength();
    }

    int getVersion() const {
        return version;
    }

    char* getAddress() const {
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