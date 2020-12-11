#pragma once

#include "WinSock.h"

class WinSockTCP : public WinSock {
public:
    WinSockTCP(Proxy* proxy, const char* host, unsigned short port);

    void start() override;

    bool send(const void* address, const char payload[], unsigned int size) override;

    bool createSocket(Player* player, Server* target) override;
};