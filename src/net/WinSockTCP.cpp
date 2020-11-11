#include "net/WinSockTCP.h"

#include <iostream>
#include <list>

WinSockTCP::WinSockTCP(Proxy* proxy, unsigned short port) : WinSock(proxy) {
    setup(port, SOCK_STREAM, IPPROTO_TCP);

    u_long nonBlock = 1;

    if (ioctlsocket(server, FIONBIO, &nonBlock) != NO_ERROR) {
        std::cout << "TCP socket failed nonblocking" << std::endl;
    }
}

void WinSockTCP::start() {
    SOCKET newConnection;
    sockaddr clientaddr{};
    std::list<SOCKET> connections;

    //char error_code;
    //int error_code_size = sizeof(error_code);
    char buffer[1024] = {0};

    TIMEVAL timeout{0, 1000};

    while (proxy->isRunning()) {
        newConnection = accept(server, &clientaddr, &addrlen);

        if (newConnection != INVALID_SOCKET) {
            std::cout << "New connection!" << std::endl;
            connections.push_back(newConnection);
        }

        auto iter = connections.begin();
        auto end = connections.end();

        while (iter != end) {
            SOCKET connection = *iter;
            fd_set readable;

            FD_ZERO(&readable);
            FD_SET(connection, &readable);

            int result = select(NULL, &readable, nullptr, nullptr, nullptr);

            if (result == SOCKET_ERROR) {
                std::cout << "TCP Select error!" << std::endl;
                continue;
            }

            if (!FD_ISSET(connection, &readable)) {
                continue;
            }

            int bytes = recv(connection, buffer, sizeof(buffer), 0);

            if (bytes == 0) {
                std::cout << "Disconnected" << std::endl;
                iter = connections.erase(iter);
                proxy->getJavaPacketHandler().disconnect(&connection);
                continue;
            }

            iter++;

            ByteBuffer* packet = ByteBuffer::allocateBuffer(bytes, true);

            memcpy(packet->getBuffer(), buffer, bytes);
            proxy->getJavaPacketHandler().handle(&connection, packet);
        }
    }
}

void WinSockTCP::send(const void* address, const char payload[], unsigned short size) {
    ::send(*((SOCKET*) address), payload, size, 0);
}