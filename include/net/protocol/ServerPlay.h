#pragma once

#include "Protocol.h"

class ServerPlay : public Protocol {
private:
    ServerPlay() : Protocol(255) {

    }

public:
    static ServerPlay* protocol() {
        static ServerPlay protocol;

        return &protocol;
    }
};