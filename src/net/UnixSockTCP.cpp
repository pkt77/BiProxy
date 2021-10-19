#include "net/UnixSockTCP.h"

#include <iostream>
#include <list>
#include <future>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <vector>

std::list<Connection*> connections;
std::vector<pollfd> sockets;

UnixSockTCP::UnixSockTCP(Proxy* proxy, const char* host, unsigned short port) : UnixSock(proxy) {
    setup(host, port, SOCK_STREAM, IPPROTO_TCP);

    int flags = fcntl(server, F_GETFL, 0) | O_NONBLOCK;

    if ((flags = fcntl(server, F_SETFL, flags)) == -1) {
        std::cout << "TCP socket failed nonblocking " << flags << std::endl;
        exit(flags);
    }

    sockets.push_back(pollfd{server, POLLIN});
}

void UnixSockTCP::start() {
    int newConnection;
    sockaddr_in clientaddr{};
    ByteBuffer* buffer;

    while (proxy->isRunning()) {
        int polled = poll(&sockets.front(), sockets.size(), 100);

        if (polled == 0) {
            continue;
        }

        if (polled == -1) {
            std::cout << "poll error " << polled << std::endl;
            exit(polled);
        }

        auto sockIter = sockets.begin();

        // Check for incoming connections
        if ((*sockIter).revents & POLLIN) {
            newConnection = accept(server, (struct sockaddr*) &clientaddr, (socklen_t*) (&addrlen));

            if (newConnection != -1) {
                sockets.push_back(pollfd{newConnection, POLLIN});
                connections.push_back(new Connection{&newConnection, ByteBuffer::allocateBuffer(1024), false, false, nullptr});
            }
        }

        sockIter++;
        auto conIter = connections.begin();

        // Check all the other sockets
        while (conIter != connections.end()) {
            pollfd socket = *sockIter;

            if (!(socket.revents & POLLIN)) {
                conIter++;
                sockIter++;
                continue;
            }

            Connection* connection = *conIter;

            if (connection->processing) {
                conIter++;
                sockIter++;
                continue;
            }

            buffer = connection->buffer;
            ssize_t bytes = recv(socket.fd, buffer->getBuffer() + buffer->getSize(), buffer->getBufferSize() - buffer->getSize(), 0);

            if (bytes == -1) {
                conIter = connections.erase(conIter);
                sockIter = sockets.erase(sockIter);
                continue;
            }

            if (bytes == 0) {
                conIter = connections.erase(conIter);
                sockIter = sockets.erase(sockIter);

                proxy->getJavaPacketHandler().disconnect(connection);

                if (connection->owner != nullptr) {
                    if (connection->owner->connectingSocket != nullptr) {
                        close(socket.fd);
                    }

                    if (connection->owner->getSocket() == connection->socket) {
                        delete connection->owner;
                    }
                }

                delete connection;
                continue;
            }

            conIter++;
            sockIter++;

            buffer->setSize(buffer->getSize() + bytes);
            connection->processing = true;

            proxy->getJavaPacketHandler().handle(connection);
            //std::async(std::launch::async, &JavaPacketHandler::handle, proxy->getJavaPacketHandler(), connection);
        }
    }
}

bool UnixSockTCP::send(const void* address, const char payload[], unsigned int size) {
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return ::send(*((int*) address), payload, size, MSG_DONTROUTE) != -1;
}

bool UnixSockTCP::createSocket(Player* player, Server* target) {
    addrinfo serverAddr{};
    addrinfo* result;

    bzero(&serverAddr, sizeof(serverAddr));

    int sock = socket(serverAddr.ai_family = AF_INET, serverAddr.ai_socktype = SOCK_STREAM, serverAddr.ai_protocol = IPPROTO_TCP);
    int addrError = getaddrinfo(target->getHost().c_str(), std::to_string(target->getPort()).c_str(), &serverAddr, &result);
    int connectError = connect(sock, result->ai_addr, result->ai_addrlen);

    int* heapSock = new int(sock);

    if (addrError == 0 && connectError == 0) {
        player->connectingSocket = heapSock;
        sockets.push_back(pollfd{sock, POLLIN});
        connections.push_back(new Connection{player->connectingSocket, ByteBuffer::allocateBuffer(250000), false, true, player});
        std::cout << player->getUsername() << " -> " << target->getName() << sock << std::endl;
        return true;
    }

    std::cout << "Failed connection " << addrError << ' ' << connectError << std::endl;
    return false;
}