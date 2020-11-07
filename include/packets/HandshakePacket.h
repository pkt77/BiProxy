#pragma once

#include "Packet.h"

class HandshakePacket : public Packet {
private:
    static constexpr char FML[] = {0, 'F', 'M', 'L', 0};

    unsigned int version;
    char* address;
    unsigned short port;
    bool isForge;
    unsigned char state;

public:
    HandshakePacket(const char* buffer) : Packet(buffer) {
        version = readVarInt();
        address = readString();
        port = readUnsignedShort();
        state = readVarInt();
    }

    unsigned int getVersion() const {
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