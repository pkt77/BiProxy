#include "utils/ByteBuffer.h"

#include <iostream>
#include <list>
#include <mutex>
#include "utils/Utils.h"

std::list<ByteBuffer*> POOL;
std::mutex POOL_LOCK;

ByteBuffer* ByteBuffer::allocateBuffer(unsigned int size, bool copy) {
    ByteBuffer* buff = nullptr;

    POOL_LOCK.lock();

    for (auto temp : POOL) {
        if (temp != nullptr && temp->released && temp->bufferSize >= size) {
            buff = temp;
            break;
        }
    }

    if (buff == nullptr) {
        buff = new ByteBuffer(size);
        POOL.push_back(buff);
    } else {
        buff->use();
    }

    POOL_LOCK.unlock();

    if (copy) {
        buff->ensureWritable(buff->bufferSize - size);
        buff->size = size;
    }

    return buff;
}

ByteBuffer::ByteBuffer(const char buffer[], unsigned int size) : buffer(new char[size]), size(size), bufferSize(size) {
    memcpy(this->buffer, buffer, size);
}

void ByteBuffer::ensureWritable(unsigned short bytes) {
    if (size + bytes >= bufferSize) {
        char* newBuff = new char[bufferSize + bytes + BUFFER_SIZE];

        memcpy(newBuff, buffer, size);

        delete[] buffer;
        buffer = newBuff;

        bufferSize += BUFFER_SIZE + bytes;
    }
}

void ByteBuffer::prefixLength() {
    unsigned char bytes = varIntLength(size);

    ensureWritable(bytes);

    for (int i = size - 1; i >= 0; i--) {
        buffer[i + bytes] = buffer[i];
    }

    offset = 0;
    writeVarInt(size);
    offset = 0;
}

unsigned char ByteBuffer::readUnsignedByte() {
    if (offset == size) {
        throw "Cannot read anymore";
    }
    return buffer[offset++];
}

void ByteBuffer::writeByte(char value) {
    ensureWritable(1);
    buffer[offset++] = value;
    size++;
}

void ByteBuffer::writeBytes(const char* bytes, unsigned short length) {
    ensureWritable(length);
    memcpy(buffer + offset, bytes, length);
    offset += length;
    size += length;
}

short ByteBuffer::readShort() {
    return readUnsignedShort();
}

unsigned short ByteBuffer::readUnsignedShort() {
    return (readUnsignedByte() << 8) + readUnsignedByte();
}

void ByteBuffer::writeUnsignedShort(unsigned short value) {
    ensureWritable(2);
    writeByte(value >> 8);
    writeByte(value);
}

void ByteBuffer::writeShort(short value) {
    writeUnsignedShort(value);
}

int ByteBuffer::readVarInt() {
    char read;
    char numRead = 0;
    int result = 0;

    do {
        read = readByte();
        result |= (read & 0b01111111) << (7 * numRead);

        if (++numRead > 5) {
            throw "VarInt is too big";
        }
    } while ((read & 0x80) == 0x80);

    return result;
}

void ByteBuffer::writeVarInt(int value) {
    do {
        char temp = (char) (value & 0b01111111);

        value = (unsigned int) value >> 7;

        if (value != 0) {
            temp |= 0b10000000;
        }
        writeByte(temp);
    } while (value != 0);
}

unsigned int ByteBuffer::readInt24bit() {
    return ((readByte() & 255)) +
           ((readByte() & 255) << 8) +
           ((readByte() & 255) << 16);
}

void ByteBuffer::writeInt24bit(unsigned int value) {
    writeByte(value);
    writeByte(value >> 8);
    writeByte(value >> 16);
}

int ByteBuffer::readInt() {
    return ((readByte() & 255) << 24) +
           ((readByte() & 255) << 16) +
           ((readByte() & 255) << 8) +
           (readByte() & 255);
}

int ByteBuffer::readIntLE() {
    return (readByte() & 255) +
           ((readByte() & 255) << 8) +
           ((readByte() & 255) << 16) +
           ((readByte() & 255) << 24);
}

void ByteBuffer::writeInt(int value) {
    ensureWritable(4);
    writeByte((unsigned int) value >> 24);
    writeByte((unsigned int) value >> 16);
    writeByte((unsigned int) value >> 8);
    writeByte((unsigned int) value);
}

long long ByteBuffer::readLong() {
    return (((long long) readByte() << 56) +
            ((long long) (readByte() & 255) << 48) +
            ((long long) (readByte() & 255) << 40) +
            ((long long) (readByte() & 255) << 32) +
            ((long long) (readByte() & 255) << 24) +
            ((readByte() & 255) << 16) +
            ((readByte() & 255) << 8) +
            ((readByte() & 255) << 0));
}

void ByteBuffer::writeLong(long long value) {
    ensureWritable(8);
    writeByte((unsigned long long) value >> 56);
    writeByte((unsigned long long) value >> 48);
    writeByte((unsigned long long) value >> 40);
    writeByte((unsigned long long) value >> 32);
    writeByte((unsigned long long) value >> 24);
    writeByte((unsigned long long) value >> 16);
    writeByte((unsigned long long) value >> 8);
    writeByte((unsigned long long) value);
}

void ByteBuffer::release() {
    offset = 0;
    size = 0;
    released = true;
}