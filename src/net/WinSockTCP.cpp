#include "net/WinSockTCP.h"

#include <iostream>
#include <list>
#include <future>

std::list<Connection*> connections;

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

    //char error_code;
    //int error_code_size = sizeof(error_code);
    int size = 900000000;
    char* buffer = new char[size];
    TIMEVAL timeout{0, 1000};

    while (proxy->isRunning()) {
        newConnection = accept(server, &clientaddr, &addrlen);

        if (newConnection != INVALID_SOCKET) {
            connections.push_back(new Connection{reinterpret_cast<void*>(newConnection), false, nullptr});
        }

        auto iter = connections.begin();
        // auto end = connections.end();

        while (iter != connections.end()) {
            Connection* connection = *iter;
            SOCKET socket = (SOCKET) connection->socket;
            fd_set readable;

            FD_ZERO(&readable);
            FD_SET(socket, &readable);

            int result = select(NULL, &readable, nullptr, nullptr, &timeout);

            if (result == SOCKET_ERROR) {
                std::cout << "TCP Select error!" << std::endl;
                iter = connections.erase(iter);
                continue;
            }

            if (!FD_ISSET(socket, &readable)) {
                iter++;
                continue;
            }

            int bytes = recv(socket, buffer, size, 0);

            if (bytes == 0) {
                iter = connections.erase(iter);

                if (connection->owner == nullptr || connection->socket == connection->owner->getSocket()) {
                    proxy->getJavaPacketHandler().disconnect(connection);

                    if (connection->owner != nullptr && connection->owner->connectingSocket != nullptr) {
                        closesocket((SOCKET) connection->owner->connectingSocket);
                    }
                } else {
                    proxy->getJavaServerPacketHandler().disconnect(connection);
                }

                if (connection->owner != nullptr && connection->owner->getSocket() == connection->socket) {
                    delete connection->owner;
                }
                delete connection;

                continue;
            }

            iter++;

            ByteBuffer* packet = ByteBuffer::allocateBuffer(bytes, true);

            memcpy(packet->getBuffer(), buffer, bytes);

            if (connection->owner == nullptr || connection->socket == connection->owner->getSocket()) {
                std::async(std::launch::async, &JavaPacketHandler::handle, proxy->getJavaPacketHandler(), connection, packet);
                //proxy->getJavaPacketHandler().handle(connection, packet);
            } else {
                std::async(std::launch::async, &JavaServerPacketHandler::handle, proxy->getJavaServerPacketHandler(), connection, packet);
                //proxy->getJavaServerPacketHandler().handle(connection, packet);
            }
        }
    }

    delete[] buffer;
}

bool WinSockTCP::send(const void* address, const char payload[], unsigned int size) {
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return ::send((SOCKET) address, payload, size, 0) != SOCKET_ERROR;
}

bool WinSockTCP::createSocket(Player* player, Server* target) {
    addrinfo hints;
    addrinfo* result;

    ZeroMemory(&hints, sizeof(hints));

    SOCKET sock = socket(hints.ai_family = AF_UNSPEC, hints.ai_socktype = SOCK_STREAM, hints.ai_protocol = IPPROTO_TCP);
    int addrError = getaddrinfo(target->getHost().c_str(), std::to_string(target->getPort()).c_str(), &hints, &result);
    int connectError = connect(sock, result->ai_addr, result->ai_addrlen);

    if (addrError == NO_ERROR && connectError == NO_ERROR) {
        player->connectingSocket = reinterpret_cast<void*>(sock);
        connections.push_back(new Connection{player->connectingSocket, true, player});
        std::cout << "Connected to server" << std::endl;
        return true;
    }

    std::cout << "Failed connection " << addrError << ' ' << connectError << std::endl;
    return false;
}