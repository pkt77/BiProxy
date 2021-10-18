#include "net/UnixSock.h"

#include <iostream>
#include <unistd.h>

void UnixSock::setup(const char* host, unsigned short port, int type, int protocol) {
    server = socket(AF_INET, type, protocol);

    if (server == -1) {
        std::cout << "Socket failed with error " << std::endl;
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);

    if (bind(server, (sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cout << "Socket failed to bind with error " << std::endl;
        close(server);
        return;
    }

    listen(server, 5);
    std::cout << "Proxy listening on " << host << ":" << port << std::endl;
}

void UnixSock::stop() {
    close(server);
    std::cout << "Killing UnixSock " << ntohs(addr.sin_port) << std::endl;
}