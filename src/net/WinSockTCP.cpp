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
    TIMEVAL timeout{0, 100};
    ByteBuffer* buffer;

    while (proxy->isRunning()) {
        newConnection = accept(server, &clientaddr, &addrlen);

        if (newConnection != INVALID_SOCKET) {
            connections.push_back(new Connection{reinterpret_cast<void*>(newConnection), ByteBuffer::allocateBuffer(1024), false, false, nullptr});
        }

        auto iter = connections.begin();

        while (iter != connections.end()) {
            Connection* connection = *iter;

            if (connection->processing) {
                iter++;
                continue;
            }

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

            buffer = connection->buffer;
            int bytes = recv(socket, buffer->getBuffer() + buffer->getSize(), buffer->getBufferSize() - buffer->getSize(), 0);

            if (bytes == SOCKET_ERROR) {
                std::cout << "RECV error " << WSAGetLastError() << std::endl;
                iter = connections.erase(iter);
                continue;
            }

            if (bytes == 0) {
                iter = connections.erase(iter);

                proxy->getJavaPacketHandler().disconnect(connection);

                if (connection->owner != nullptr) {
                    if (connection->owner->connectingSocket != nullptr) {
                        closesocket((SOCKET) connection->owner->connectingSocket);
                    }

                    if (connection->owner->getSocket() == connection->socket) {
                        delete connection->owner;
                    }
                }

                delete connection;
                continue;
            }

            iter++;

            buffer->setSize(buffer->getSize() + bytes);
            connection->processing = true;

            std::async(std::launch::async, &JavaPacketHandler::handle, proxy->getJavaPacketHandler(), connection);
        }
    }
}

bool WinSockTCP::send(const void* address, const char payload[], unsigned int size) {
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return ::send((SOCKET) address, payload, size, MSG_DONTROUTE) != SOCKET_ERROR;
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
        connections.push_back(new Connection{player->connectingSocket, ByteBuffer::allocateBuffer(250000), false, true, player});
        std::cout << "Connected to server" << std::endl;
        return true;
    }

    std::cout << "Failed connection " << addrError << ' ' << connectError << std::endl;
    return false;
}