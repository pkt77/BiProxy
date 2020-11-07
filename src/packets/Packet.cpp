#include "packets/Packet.h"

Packet::Packet() : size(0) {
    buffer = new char[0];
}

Packet::Packet(const char buffer[]) : buffer(const_cast<char*>(buffer)) {
    size = static_cast<unsigned int>(readVarInt());
    id = readByte();
}

void Packet::writeByte(char value) {
    if (offset >= size) {
        char* newBuff = new char[size + BUFFER_SIZE];

        std::memcpy(newBuff, buffer, size);

        for (unsigned int i = size; i < size + BUFFER_SIZE; i++) {
            newBuff[i] = '\0';
        }

        delete[] buffer;
        buffer = &newBuff[0];

        size += BUFFER_SIZE;
    }

    buffer[offset++] = value;
    realSize++;
}

short Packet::readShort() {
    return(readByte() << 8) + readByte();
}

unsigned short Packet::readUnsignedShort() {
    return (readByte() << 8) + readByte();
}

void Packet::writeShort(short value) {
    writeByte((unsigned short) value >> 8);
    writeByte((unsigned short) value);
}

int Packet::readVarInt() {
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

void Packet::writeVarInt(int value) {
    do {
        // std::cout << value << std::endl;
        char temp = (char) (value & 0b01111111);

        value = (unsigned int) value >> 7;

        if (value != 0) {
            temp |= 0b10000000;
        }
        // std::cout << +temp << std::endl;
        writeByte(temp);
    } while (value != 0);
}

long long Packet::readLong() {
    return (((long long) readByte() << 56) +
            ((long long) (readByte() & 255) << 48) +
            ((long long) (readByte() & 255) << 40) +
            ((long long) (readByte() & 255) << 32) +
            ((long long) (readByte() & 255) << 24) +
            ((readByte() & 255) << 16) +
            ((readByte() & 255) << 8) +
            ((readByte() & 255) << 0));
}

void Packet::writeLong(long long value) {
    writeByte((unsigned long long) value >> 56);
    writeByte((unsigned long long) value >> 48);
    writeByte((unsigned long long) value >> 40);
    writeByte((unsigned long long) value >> 32);
    writeByte((unsigned long long) value >> 24);
    writeByte((unsigned long long) value >> 16);
    writeByte((unsigned long long) value >> 8);
    writeByte((unsigned long long) value);
}

char* Packet::readString() {
    int length = readVarInt();
    char* string = new char[length];

    for (int i = 0; i < length; i++) {
        string[i] = readByte();
    }

    string[length] = 0;
    return string;
}

void Packet::writeString(const std::string& value, bool varInt) {
    if (varInt) {
        writeVarInt(value.length());
    } else {
        writeShort(value.length());
    }

    for (char i : value) {
        writeByte(i);
    }
}

void Packet::writeString(const char* value) {
    int length = 0;

    while (value[length] != '\0') {
        length++;
    }

    if (length == 0) {
        return;
    }

    writeVarInt(length);

    for (int i = 0; i < length; i++) {
        writeByte(value[i]);
    }
}
