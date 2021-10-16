#pragma once

#include <utils/Utils.h>
#include <utils/ByteBuffer.h>
#include "BedrockClientHandler.h"

class Proxy;

class RakNetPacketHandler {
private:
    static constexpr char MAGIC[] = {0, -1, -1, 0, -2, -2, -2, -2, -3, -3, -3, -3, 18, 52, 86, 120};
    static constexpr unsigned char INTERNAL_ADDRESSES[] = {4, 128, 255, 255, 254, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188,
                                                           4, 255, 255, 255, 255, 74, 188};

    Proxy* proxy;
    BedrockClientHandler bedrockClientHandler;

    std::string motd;

public:
    RakNetPacketHandler(Proxy* proxy) : proxy(proxy), bedrockClientHandler(BedrockClientHandler(proxy)) {}

    void handle(const void* socket, unsigned char address[], unsigned short port, ByteBuffer* packet) const;

    void sendEncapsulated(const void* socket, ByteBuffer* packet, unsigned char reliability, bool deflate) const;

    void setMotd(const std::string& line1, const std::string& line2);
};