#pragma once

#include "net/WinSock.h"

class WinSockUDP : public WinSock {
public:
    WinSockUDP(Proxy* proxy, unsigned short port);

    void start() override;

    void send(const void* address, const char payload[], unsigned short size) override;
};