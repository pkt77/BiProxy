#pragma once

#include <string>

#include <thread>
#include <net/JavaPacketHandler.h>
#include <net/RakNetPacketHandler.h>
#include <data/Server.h>
#include "net/AbstractSocket.h"
#include <map>
#include <data/JavaPing.h>

class Proxy {
private:
    bool running = true;

    AbstractSocket* jeSocket;
    AbstractSocket* beSocket;

    std::thread jeThread;
    std::thread beThread;

    JavaPacketHandler javaPacketHandler;
    RakNetPacketHandler rakNetPacketHandler;

    JavaPing defaultPing;

    std::map<std::string, Server*> servers;

public:
    Proxy();

    Server* createServer(std::string name, std::string host, unsigned short port);

    Server* getServer(std::string& name);

    void shutdown();

    const bool& isRunning() const {
        return running;
    }

    AbstractSocket* getJeSocket() const {
        return jeSocket;
    }

    AbstractSocket* getBeSocket() const {
        return beSocket;
    }

    const JavaPacketHandler& getJavaPacketHandler() const {
        return javaPacketHandler;
    }

    const RakNetPacketHandler& getRakNetPacketHandler() const {
        return rakNetPacketHandler;
    }

    JavaPing& getDefaultPing() {
        return defaultPing;
    }
};