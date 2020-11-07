#pragma once

#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <string>
#include "Proxy.h"

class WinSock : public AbstractSocket {
protected:
    Proxy* proxy;

    SOCKADDR_IN addr;
    int addrlen = sizeof(addr);

    SOCKET server;

    void setup(unsigned short port, int type, int protocol);

public:
    WinSock(Proxy* proxy) : proxy(proxy) {}

    void stop() override;
};