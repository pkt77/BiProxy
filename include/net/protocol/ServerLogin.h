#pragma once

#include "Protocol.h"

#define DISCONNECT 0
#define LOGIN_SUCCESS 2

class ServerLogin : public Protocol {
private:
    ServerLogin() : Protocol(5) {
        setHandler(DISCONNECT, &disconnect);
        setHandler(LOGIN_SUCCESS, &loginSuccess);
    }

    static bool disconnect(Connection*);

    static bool loginSuccess(Connection*);

public:
    static ServerLogin* protocol() {
        static ServerLogin protocol;

        return &protocol;
    }
};