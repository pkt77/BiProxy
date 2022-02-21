#include "net/WinSockTCP.h"

#include <iostream>
#include <list>
#include <future>

#include "net/protocol/Handshake.h"
#include "net/protocol/ServerLogin.h"

std::list<Connection*> connections;

WinSockTCP::WinSockTCP(Proxy* proxy, const char* host, unsigned short port) : WinSock(proxy) {
    setup(host, port, SOCK_STREAM, IPPROTO_TCP);

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
        newConnection = accept(server, &clientaddr, (int*) &address->ai_addrlen);

        if (newConnection != INVALID_SOCKET) {
            connections.push_back(new Connection{reinterpret_cast<void*>(newConnection), Handshake::protocol(), ByteBuffer::allocateBuffer(1024),
                                                 false, STATE_HANDSHAKE, nullptr});
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
    addrinfo* host;
    int addrError = getaddrinfo(target->getHost().c_str(), nullptr, nullptr, &host);

    if (addrError != NO_ERROR) {
        std::cout << "Failed to find host " << target->getHost() << ' ' << addrError << std::endl;
        return false;
    }

    if (host->ai_next != nullptr) {
        host = host->ai_next;
    }

    ((sockaddr_in*) host->ai_addr)->sin_port = htons(target->getPort());

    SOCKET sock = socket(host->ai_family, host->ai_socktype, host->ai_protocol);

    if (connect(sock, host->ai_addr, (int) host->ai_addrlen) == NO_ERROR) {
        player->connectingSocket = reinterpret_cast<void*>(sock);
        connections.push_back(new Connection{player->connectingSocket, ServerLogin::protocol(), ByteBuffer::allocateBuffer(250000),
                                             false, STATE_LOGIN, player});
        std::cout << "Connected to server" << std::endl;
        return true;
    }

    std::cout << "Failed connection " << WSAGetLastError() << std::endl;
    return false;
}