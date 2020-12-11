#include "net/WinSockUDP.h"

WinSockUDP::WinSockUDP(Proxy* proxy, const char* host, unsigned short port) : WinSock(proxy) {
    setup(host, port, SOCK_DGRAM, IPPROTO_UDP);
}

void WinSockUDP::start() {
    char buffer[8192] = {0};
    SOCKADDR_IN sender;
    int addrSize = sizeof(sender);

    while (proxy->isRunning()) {
        int bytes = recvfrom(server, buffer, sizeof(buffer), 0, (SOCKADDR*) &sender, &addrSize);

        if (bytes < 1) {
            continue;
        }

        ByteBuffer* packet = ByteBuffer::allocateBuffer(bytes, true);
        unsigned char address[] = {sender.sin_addr.S_un.S_un_b.s_b1, sender.sin_addr.S_un.S_un_b.s_b2, sender.sin_addr.S_un.S_un_b.s_b3,
                                   sender.sin_addr.S_un.S_un_b.s_b4};

        memcpy(packet->getBuffer(), buffer, bytes);
        proxy->getRakNetPacketHandler().handle(&sender, address, sender.sin_port, packet);
    }
}

bool WinSockUDP::send(const void* address, const char* payload, unsigned int size) {
    return sendto(server, payload, size, 0, (SOCKADDR*) address, addrlen) != SOCKET_ERROR;
}

bool WinSockUDP::createSocket(Player* player, Server* target) {
    return false;
}