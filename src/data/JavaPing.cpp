#include "data/JavaPing.h"

std::string JavaPing::toJSON() const {
    std::string json(
            R"({"version":{"name":")" + name + R"(","protocol":)" + std::to_string(protocolVersion) + R"(},"players":{"online":)" + std::to_string(onlinePlayers) +
            R"(,"max":)" + std::to_string(maxPlayers));

    //TODO sample players
    json += '}';

    if (!description.empty()) {
        json += R"(,"description":{"text":")" + description + "\"}";
    }

    if (!iconBase64.empty()) {
        json += R"(,"favicon":"data:image/png;base64,)" + iconBase64 + "\"";
    }

    json += '}';
    return json;
}