#include <fstream>
#include <iostream>
#include "Proxy.h"

#ifdef _WIN32

#include "net/WinSockTCP.h"
#include "net/WinSockUDP.h"

#endif

Proxy::Proxy(std::string host, unsigned short port) : host(host), javaPacketHandler(this), rakNetPacketHandler(this) {
    std::string line;
    std::ifstream file("motd.json");

    while (getline(file, line)) {
        motdString += line;
        std::cout << line << std::endl;
    }
    file.close();

#ifdef _WIN32
    jeSocket = new WinSockTCP(this, 25565);
    beSocket = new WinSockUDP(this, 19132);

    jeThread = std::thread(&WinSockTCP::start, (WinSockTCP*) jeSocket);
    beThread = std::thread(&WinSockUDP::start, (WinSockUDP*) beSocket);
#else
    //Unix sockets
#endif
}

void Proxy::shutdown() {
    running = false;

    jeSocket->stop();
    beSocket->stop();

    jeThread.join();
    beThread.join();
}