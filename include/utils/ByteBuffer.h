#pragma once

#include <string>

#define BUFFER_SIZE 50

class ByteBuffer {
private:
    char* buffer;

    unsigned int bufferSize;
    unsigned int offset = 0;
    unsigned int size;

    bool released = false;

public:
    static ByteBuffer* allocateBuffer(unsigned int size, bool copy = false);

    ByteBuffer(unsigned int size) : size(0), bufferSize(size), buffer(new char[size]) {}

    ByteBuffer(const char buffer[], unsigned int size);

    ~ByteBuffer() {
        delete[] buffer;
    }

    void ensureWritable(unsigned short bytes);

    void prefixLength();

    static unsigned char flip(unsigned char byte) {
        return ~byte & 0xFF;
    }

    char readByte() {
        return buffer[offset++];
    }

    unsigned char readUnsignedByte() {
        return buffer[offset++];
    }

    void writeByte(char value);

    void writeUnsignedByte(unsigned char value) {
        writeByte(value);
    }

    void writeBytes(const char* bytes, unsigned short length);

    void writeBytes(const unsigned char* bytes, unsigned short length) {
        writeBytes((const char*) bytes, length);
    };

    bool readBoolean() {
        return buffer[offset++];
    }

    void writeBoolean(bool value) {
        writeByte(value);
    }

    short readShort();

    void writeShort(short value);

    unsigned short readUnsignedShort();

    void writeUnsignedShort(unsigned short value);

    int readVarInt();

    void writeVarInt(int value);

    unsigned int readInt24bit();

    void writeInt24bit(unsigned int value);

    int readInt();

    void writeInt(int value);

    long long readLong();

    void writeLong(long long value);

    char* readString();

    void writeString(const std::string& value, bool varInt);

    void writeString(const char* value, bool varInt);

    void release();

    void use() {
        released = false;
    }

    int readableBytes() const {
        return (int) size - (int) offset;
    }

    bool isReadable() const {
        return readableBytes() > 0;
    }

    char* getBuffer() const {
        return buffer;
    }

    unsigned int getOffset() const {
        return offset;
    }

    unsigned int getSize() const {
        return size;
    }

    void setOffset(unsigned int offset) {
        this->offset = offset;
    }
};