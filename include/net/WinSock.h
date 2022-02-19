#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>
#include "Proxy.h"

class WinSock : public AbstractSocket {
protected:
    Proxy* proxy;

    addrinfo* address;

    SOCKET server;

    void setup(const char* host, unsigned short port, int type, int protocol);

public:
    WinSock(Proxy* proxy) : proxy(proxy) {}

    void stop() override;
};