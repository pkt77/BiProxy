#pragma once

#include <string>

#define BUFFER_SIZE 50

class ByteBuffer {
private:
    char* buffer;

    unsigned int bufferSize;
    unsigned int offset = 0;
    unsigned short size;

    bool released = false;

public:
    ByteBuffer();

    ByteBuffer(const char buffer[], unsigned short size) : buffer(const_cast<char*>(buffer)), size(size), bufferSize(size) {}

    void ensureWritable(unsigned short bytes);

    void prefixLength();

    char readByte() {
        return buffer[offset++];
    }

    void writeByte(char value);

    void writeBytes(const char* bytes, unsigned short length);

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

    void writeString(const char* value, bool varInt);

    void release();

    bool isReadable() const {
        return (int) size - (int) offset > 0;
    }

    char* getBuffer() const {
        return buffer;
    }

    unsigned int getOffset() const {
        return offset;
    }

    unsigned short getSize() const {
        return size;
    }

    void setOffset(unsigned int offset) {
        this->offset = offset;
    }
};