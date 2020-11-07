#pragma once

#include "Packet.h"

class CompressionPacket : Packet {
private:
    int threshold;

public:
    CompressionPacket(const char buffer[]) : Packet(buffer) {
        threshold = readVarInt();
    }

    int getThreshold() const {
        return threshold;
    }
};