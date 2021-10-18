#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>
#include "Proxy.h"

class UnixSock : public AbstractSocket {
protected:
    Proxy* proxy;

    sockaddr_in addr;
    int addrlen = sizeof(addr);

    int server;

    void setup(const char* host, unsigned short port, int type, int protocol);

public:
    UnixSock(Proxy* proxy) : proxy(proxy) {}

    void stop() override;
};