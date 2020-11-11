#pragma once

#include "Packet.h"

class CompressionPacket : Packet {
private:
    int threshold;

public:
    void read(ByteBuffer* buffer) override {
        threshold = buffer->readVarInt();
    }

    void write(ByteBuffer* buffer) override {

    }

    int getThreshold() const {
        return threshold;
    }
};