#pragma once

#include "WinSock.h"

class WinSockTCP : public WinSock {
public:
    WinSockTCP(Proxy* proxy, unsigned short port);

    void start() override;

    void send(const void* address, const char payload[], unsigned short size) override;
};