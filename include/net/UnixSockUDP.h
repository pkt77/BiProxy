#pragma once

#include "net/UnixSock.h"

class UnixSockUDP : public UnixSock {
public:
    UnixSockUDP(Proxy* proxy, const char* host, unsigned short port);

    void start() override;

    bool send(const void* address, const char payload[], unsigned int size) override;

    bool createSocket(Player* player, Server* target) override;
};