#include <iostream>
#include "net/UnixSockUDP.h"

UnixSockUDP::UnixSockUDP(Proxy* proxy, const char* host, unsigned short port) : UnixSock(proxy) {
    setup(host, port, SOCK_DGRAM, IPPROTO_UDP);
}

void UnixSockUDP::start() {
    char buffer[8192] = {0};
    sockaddr_in sender;
    int addrSize = sizeof(sender);

    while (proxy->isRunning()) {
        int bytes = recvfrom(server, buffer, sizeof(buffer), 0, (sockaddr*) &sender, (socklen_t*) (&addrSize));

        if (bytes < 1) {
            continue;
        }

        ByteBuffer* packet = ByteBuffer::allocateBuffer(bytes, true);
        unsigned char address[] = {(unsigned char) sender.sin_addr.s_addr, (unsigned char) (sender.sin_addr.s_addr << 8),
                                   (unsigned char) (sender.sin_addr.s_addr << 16), (unsigned char) (sender.sin_addr.s_addr << 24)};

        memcpy(packet->getBuffer(), buffer, bytes);
        proxy->getRakNetPacketHandler().handle(&sender, address, sender.sin_port, packet);
    }
}

bool UnixSockUDP::send(const void* address, const char* payload, unsigned int size) {
    return sendto(server, payload, size, 0, (sockaddr*) address, addrlen) != -1;
}

bool UnixSockUDP::createSocket(Player* player, Server* target) {
    return false;
}