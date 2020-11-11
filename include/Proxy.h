#pragma once

#include <string>

#include <thread>
#include <net/JavaPacketHandler.h>
#include <net/RakNetPacketHandler.h>
#include "net/AbstractSocket.h"

class Proxy {
private:
    const std::string host;
    //unsigned short port;

    bool running = true;

    AbstractSocket* jeSocket;
    AbstractSocket* beSocket;

    std::thread jeThread;
    std::thread beThread;

    const JavaPacketHandler javaPacketHandler;
    const RakNetPacketHandler rakNetPacketHandler;

    std::string iconBase64;
    std::string motdString;

public:
    Proxy(std::string host, unsigned short port);

    void shutdown();

    const std::string& getHost() {
        return host;
    }

/*    unsigned short & getPort(){
        return port;
    }*/

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

    const std::string& getMotdString() const {
        return motdString;
    }
};