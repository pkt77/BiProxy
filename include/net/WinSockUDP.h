#pragma once

#include "net/WinSock.h"

class WinSockUDP : public WinSock {
public:
    WinSockUDP(Proxy* proxy, const char* host, unsigned short port);

    void start() override;

    bool send(const void* address, const char payload[], unsigned int size) override;

    bool createSocket(Player* player, Server* target) override;
};