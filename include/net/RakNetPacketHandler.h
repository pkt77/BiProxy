#pragma once

class Proxy;

class RakNetPacketHandler {
private:
    static constexpr char MAGIC[] = {0, -1, -1, 0, -2, -2, -2, -2, -3, -3, -3, -3, 18, 52, 86, 120};

    Proxy* proxy;

public:
    RakNetPacketHandler(Proxy* proxy) : proxy(proxy) {}

    void handle(const void* address, const char payload[], unsigned short size) const;
};