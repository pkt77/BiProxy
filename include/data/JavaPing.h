#pragma once

#include <string>

class JavaPing {
private:
    std::string name;
    int protocolVersion;

    unsigned int onlinePlayers;
    unsigned int maxPlayers;

    //TODO sample players

    std::string description;
    std::string iconBase64;

public:
    std::string toJSON() const;

    const std::string& getName() const {
        return name;
    }

    void setName(const std::string& name) {
        JavaPing::name = name;
    }

    int getProtocolVersion() const {
        return protocolVersion;
    }

    void setProtocolVersion(int protocolVersion) {
        JavaPing::protocolVersion = protocolVersion;
    }

    unsigned int getOnlinePlayers() const {
        return onlinePlayers;
    }

    void setOnlinePlayers(unsigned int onlinePlayers) {
        JavaPing::onlinePlayers = onlinePlayers;
    }

    unsigned int getMaxPlayers() const {
        return maxPlayers;
    }

    void setMaxPlayers(unsigned int maxPlayers) {
        JavaPing::maxPlayers = maxPlayers;
    }

    const std::string& getDescription() const {
        return description;
    }

    void setDescription(const std::string& description) {
        JavaPing::description = description;
    }

    const std::string& getIconBase64() const {
        return iconBase64;
    }

    void setIconBase64(const std::string& iconBase64) {
        JavaPing::iconBase64 = iconBase64;
    }
};