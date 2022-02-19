#include "net/WinSock.h"

#include <iostream>

void WinSock::setup(const char* host, unsigned short port, int type, int protocol) {
    WSAData wsaData{};

    if (WSAStartup(WINSOCK_VERSION, &wsaData) != NO_ERROR) {
        return;
    }

    int err = getaddrinfo(host, nullptr, nullptr, &address);

    if (err != NO_ERROR) {
        std::cout << "Failed to get address " << host << " code " << err << std::endl;
        WSACleanup();
        return;
    }

    if (address->ai_next != nullptr) {
        address = address->ai_next;
    }

    ((sockaddr_in*) address->ai_addr)->sin_port = htons(port);

    server = socket(AF_INET, type, protocol);

    if (server == INVALID_SOCKET) {
        std::cout << "Socket failed with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    if (bind(server, address->ai_addr, (int) address->ai_addrlen) == SOCKET_ERROR) {
        std::cout << "Socket failed to bind with error " << WSAGetLastError() << std::endl;
        stop();
        return;
    }

    listen(server, SOMAXCONN);

    std::cout << "Proxy listening on " << host << ":" << port << std::endl;
}

void WinSock::stop() {
    std::cout << "Killing Winsock " << ntohs(((sockaddr_in*) address->ai_addr)->sin_port) << std::endl;
    shutdown(server, SD_BOTH);
    WSACleanup();
}