#pragma once

class AbstractSocket {
public:
    virtual void start() = 0;

    virtual void send(const void* address, Packet& packet) {
        send(address, packet.getBuffer(), packet.getRealSize());
    };

    virtual void send(const void* address, const char payload[], unsigned short size) = 0;

    virtual void stop() = 0;
};