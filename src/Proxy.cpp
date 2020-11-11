#include <fstream>
#include <iostream>
#include "Proxy.h"

#include "utils/Base64.h"

#ifdef _WIN32

#include "net/WinSockTCP.h"
#include "net/WinSockUDP.h"

#endif

Proxy::Proxy(std::string host, unsigned short port) : host(host), javaPacketHandler(this), rakNetPacketHandler(this) {
    std::string line;
    std::ifstream file("motd.json");

    while (getline(file, line)) {
        motdString += line;
    }
    file.close();

    std::ifstream icon("server-icon.png", std::ios::binary);

    icon.seekg(0, std::ios::end);
    size_t length = icon.tellg();
    icon.seekg(0, std::ios::beg);
    char* fileByes = new char[length];

    icon.read(fileByes, length);
    icon.close();

    iconBase64 = base64_encode((unsigned char*) fileByes, length);
    motdString += iconBase64;
    motdString += "\"}";
    std::cout << motdString << std::endl;

    delete[] fileByes;

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