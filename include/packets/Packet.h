#pragma once

#include <string>

class Packet {
protected:
    static const int BUFFER_SIZE = 50;

    char* buffer;
    unsigned int size;
    unsigned int offset = 0;
    unsigned short realSize = 0;

    unsigned char id;

public:
    Packet();

    Packet(const char* buffer);

    char* getBuffer() const {
        return buffer;
    }

    unsigned int getOffset() const {
        return offset;
    }

    unsigned short getRealSize() const {
        return realSize;
    }

    void setOffset(unsigned int offset) {
        this->offset = offset;
    }

    unsigned char getId() const {
        return id;
    }

    char readByte() {
        return buffer[offset++];
    }

    void writeByte(char value);

    short readShort();

    void writeShort(short value);

    unsigned short readUnsignedShort();

    void writeUnsignedShort(unsigned short value);

    int readVarInt();

    void writeVarInt(int value);

    long long readLong();

    void writeLong(long long value);

    char* readString();

    void writeString(const std::string& value, bool varInt);

    void writeString(const char* value);
};