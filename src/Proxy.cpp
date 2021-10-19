#include <fstream>
#include <iostream>
#include "Proxy.h"

#include "utils/Base64.h"
#include <yaml-cpp/yaml.h>
#include <utils/Encryption.h>

#ifdef _WIN32

#include "net/WinSockTCP.h"
#include "net/WinSockUDP.h"

#else

#include <net/UnixSockTCP.h>
#include <net/UnixSockUDP.h>

#endif

Proxy::Proxy() : javaPacketHandler(this), rakNetPacketHandler(this) {
    YAML::Node config;

    Encryption::init(id);

    try {
        config = YAML::LoadFile("config.yml");
    } catch (YAML::BadFile e) {
        std::cout << "No config file found, creating one..." << std::endl;

        std::ofstream saveConfig("config.yml");

        saveConfig << R"(Bedrock:
  Host: 0.0.0.0:19132
  MOTD Line 1: BiProxy
  MOTD Line 2: Bedrock Server
  Servers:
    hub: 127.0.0.1:19133
Java:
  Host: 0.0.0.0:25565
  MOTD Line 1: BiProxy
  MOTD Line 2: Bedrock Server
  Servers:
    hub: 127.0.0.1:25566)";
        saveConfig.close();

        config = YAML::LoadFile("config.yml");
    }

    if (config["Bedrock"] && config["Bedrock"]["Host"]) {
        std::pair<std::string, unsigned short> host = parseHost(config["Bedrock"]["Host"].as<std::string>());

        rakNetPacketHandler.setMotd(config["Bedrock"]["MOTD Line 1"].as<std::string>(), config["Bedrock"]["MOTD Line 2"].as<std::string>());

#ifdef _WIN32
        beSocket = new WinSockUDP(this, host.first.c_str(), host.second);
        beThread = std::thread(&WinSockUDP::start, (WinSockUDP*) beSocket);
#else
        beSocket = new UnixSockUDP(this, host.first.c_str(), host.second);
        beThread = std::thread(&UnixSockUDP::start, (UnixSockUDP*) beSocket);
#endif
    }

    if (config["Java"] && config["Java"]["Host"]) {
        std::pair<std::string, unsigned short> host = parseHost(config["Java"]["Host"].as<std::string>());
        YAML::Node servers = config["Java"]["Servers"];
        std::ifstream icon("server-icon.png", std::ios::binary);

        defaultPing.setName("BiProxy");
        defaultPing.setProtocolVersion(756);
        defaultPing.setOnlinePlayers(0);
        defaultPing.setMaxPlayers(1000);
        defaultPing.setDescription(config["Java"]["MOTD Line 1"].as<std::string>() + '\n' + config["Java"]["MOTD Line 2"].as<std::string>());

        if (icon.is_open()) {
            icon.seekg(0, std::ios::end);
            size_t length = icon.tellg();
            icon.seekg(0, std::ios::beg);
            char* fileByes = new char[length];

            icon.read(fileByes, length);
            icon.close();

            defaultPing.setIconBase64(base64_encode((unsigned char*) fileByes, length));

            delete[] fileByes;
        }

#ifdef _WIN32
        jeSocket = new WinSockTCP(this, host.first.c_str(), host.second);
        jeThread = std::thread(&WinSockTCP::start, (WinSockTCP*) jeSocket);
#else
        jeSocket = new UnixSockTCP(this, host.first.c_str(), host.second);
        jeThread = std::thread(&UnixSockTCP::start, (UnixSockTCP*) jeSocket);
#endif

        if (servers) {
            for (auto iter = servers.begin(); iter != servers.end(); iter++) {
                host = parseHost(iter->second.as<std::string>());

                createServer(iter->first.as<std::string>(), host.first, host.second);
            }
        }
    }
}

Server* Proxy::createServer(std::string name, std::string host, unsigned short port) {
    Server* server = new Server(name, host, port);

    servers[name] = server;

    return server;
}

Server* Proxy::getServer(std::string& name) {
    auto it = servers.find(name);

    return it == servers.end() ? nullptr : it->second;
}

void Proxy::shutdown() {
    running = false;

    jeSocket->stop();
    beSocket->stop();

    jeThread.join();
    beThread.join();
}