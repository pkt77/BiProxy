#pragma once

#include "Protocol.h"

class ClientPlay : public Protocol {
private:
    ClientPlay() : Protocol(255) {

    }

public:
    static ClientPlay* protocol() {
        static ClientPlay protocol;

        return &protocol;
    }
};