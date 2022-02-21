#pragma once

#include "Protocol.h"

#define STATUS 0
#define PING 1

class ClientStatus : public Protocol {
private:
    ClientStatus() : Protocol(3) {
        setHandler(STATUS, &request);
        setHandler(PING, &ping);
    }

    static bool request(Connection*);

    static bool ping(Connection*);

public:
    static ClientStatus* protocol() {
        static ClientStatus protocol;

        return &protocol;
    }
};