#pragma once

#include <utils/Utils.h>

class Proxy;

class RakNetPacketHandler {
private:
    static constexpr char MAGIC[] = {0, -1, -1, 0, -2, -2, -2, -2, -3, -3, -3, -3, 18, 52, 86, 120};
    static constexpr unsigned char ADDRESSES[70] = {0};

    long long GUID = randomLong();

    Proxy* proxy;

public:
    RakNetPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(const void* address, const char payload[], unsigned short size) const;
};