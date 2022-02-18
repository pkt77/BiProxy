#pragma once

#include <utils/ByteBuffer.h>

class Encryption {
public:
    static void init(long long id);

    static char* getHexId();

    static void addKey(ByteBuffer* buffer);

    static std::string genServerHash(const std::string& secret);

    static std::string decodeMessage(const std::string& secret);

    static void encode(std::string secret, ByteBuffer* buffer);
};