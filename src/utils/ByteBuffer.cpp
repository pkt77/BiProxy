#include "utils/ByteBuffer.h"

#include <iostream>
#include <list>
#include <mutex>

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

        std::memcpy(newBuff, buffer, bufferSize);

        delete[] buffer;
        buffer = newBuff;

        bufferSize += BUFFER_SIZE + bytes;
    }
}

void ByteBuffer::prefixLength() {
    unsigned char bytes;

    if ((size & 0xFFFFFF80) == 0) {
        bytes = 1;
    } else if ((size & 0xFFFFC000) == 0) {
        bytes = 2;
    } else if ((size & 0xFFE00000) == 0) {
        bytes = 3;
    } else if ((size & 0xF0000000) == 0) {
        bytes = 4;
    } else {
        bytes = 5;
    }

    ensureWritable(bytes);

    for (int i = size - 1; i >= 0; i--) {
        buffer[i + bytes] = buffer[i];
    }

    offset = 0;
    writeVarInt(size);
    offset = 0;
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
    return (readByte() << 8) + readByte();
}

unsigned short ByteBuffer::readUnsignedShort() {
    return (readByte() << 8) + readByte();
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
    int numRead = 0;
    int result = 0;
    char read;

    do {
        read = readByte();
        int value = (read & 0b01111111);
        result |= (value << (7 * numRead));

        numRead++;
        if (numRead > 5) {
            throw ("VarInt is too big");
        }
    } while ((read & 0b10000000) != 0);

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
    return ((readByte() & 255) << 16) +
           ((readByte() & 255) << 8) +
           (readByte() & 255);
}

void ByteBuffer::writeInt24bit(unsigned int value) {
    writeByte(value >> 16);
    writeByte(value >> 8);
    writeByte(value);
}

int ByteBuffer::readInt() {
    return ((readByte() & 255) << 24) +
           ((readByte() & 255) << 16) +
           ((readByte() & 255) << 8) +
           (readByte() & 255);
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

char* ByteBuffer::readString() {
    int length = readVarInt();
    char* string = new char[length];

    for (int i = 0; i < length; i++) {
        string[i] = readByte();
    }

    string[length] = 0;
    return string;
}

void ByteBuffer::writeString(const std::string& value, bool varInt) {
    unsigned short length = value.length();

    if (varInt) {
        ensureWritable(5 + length);
        writeVarInt(length);
    } else {
        ensureWritable(2 + length);
        writeShort(length);
    }

    memcpy(buffer + offset, value.data(), length);
    offset += length;
    size += length;
}

void ByteBuffer::writeString(const char* value, bool varInt) {
    size_t length = strlen(value);

    if (varInt) {
        ensureWritable(5 + length);
        writeVarInt(length);

        /*if (length > 32767) {
            std::cout << "String is too large!" << std::endl;
        }*/
    } else {
        ensureWritable(2 + length);
        writeShort(length);
    }

    memcpy(buffer + offset, value, length);
    offset += length;
    size += length;
}

void ByteBuffer::release() {
    offset = 0;
    size = 0;
    released = true;
}