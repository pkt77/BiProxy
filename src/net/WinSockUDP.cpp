#include "net/WinSockUDP.h"

WinSockUDP::WinSockUDP(Proxy* proxy, unsigned short port) : WinSock(proxy) {
    setup(port, SOCK_DGRAM, IPPROTO_UDP);
}

void WinSockUDP::start() {
    char buffer[1024] = {0};
    SOCKADDR_IN SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);

    while (proxy->isRunning()) {
        int bytes = recvfrom(server, buffer, 1024, 0, (SOCKADDR*) &SenderAddr, &SenderAddrSize);

        if (bytes < 1) {
            continue;
        }

        ByteBuffer* packet = ByteBuffer::allocateBuffer(bytes, true);

        memcpy(packet->getBuffer(), buffer, bytes);
        proxy->getRakNetPacketHandler().handle(&SenderAddr, packet);
    }
}

void WinSockUDP::send(const void* address, const char* payload, unsigned short size) {
    sendto(server, payload, size, 0, (SOCKADDR*) address, addrlen);
}