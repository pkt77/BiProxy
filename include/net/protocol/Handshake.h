#pragma once

#include "Protocol.h"

#define HANDSHAKE 0

class Handshake : public Protocol {
private:
    Handshake() : Protocol(0) {}

    static bool handshake(Connection*);

public:
    PacketHandler getHandler(unsigned char id) override {
        return &handshake;
    }

    static Handshake* protocol() {
        static Handshake protocol;

        return &protocol;
    }
};