#include "net/WinSock.h"

#include <iostream>

void WinSock::setup(const char* host, unsigned short port, int type, int protocol) {
    WSAData wsaData;

    if (WSAStartup(WINSOCK_VERSION, &wsaData) != NO_ERROR) {
        return;
    }

    InetPton(AF_INET, host, &addr.sin_addr.s_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    server = socket(AF_INET, type, protocol);

    if (server == INVALID_SOCKET) {
        std::cout << "Socket failed with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    if (bind(server, (SOCKADDR*) &addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Socket failed to bind with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    listen(server, SOMAXCONN);

    std::cout << "Proxy listening on " << host << ":" << port << std::endl;
}

void WinSock::stop() {
    std::cout << "Killing Winsock " << ntohs(addr.sin_port) << std::endl;
    shutdown(server, SD_BOTH);
    WSACleanup();
}