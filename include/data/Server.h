#pragma once

#include <string>

class Server {
private:
    const std::string name;
    const std::string host;
    const unsigned short port;

public:
    Server(std::string name, std::string host, unsigned short port) : name(name), host(host), port(port) {}

    const std::string& getName() const {
        return name;
    }

    const std::string& getHost() const {
        return host;
    }

    unsigned short getPort() const {
        return port;
    }
};