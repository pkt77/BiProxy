#pragma once

#include <utils/ByteBuffer.h>

class Packet {
public:
    virtual void read(ByteBuffer* buffer) = 0;

    virtual void write(ByteBuffer* buffer) = 0;
};