#pragma once

#include "Protocol.h"

#define LOGIN_START 0
#define ENCRYPTION_RESPONSE 1

class ClientLogin : public Protocol {
private:
    ClientLogin() : Protocol(3) {
        setHandler(LOGIN_START, &loginStart);
        setHandler(ENCRYPTION_RESPONSE, &encryptionResponse);
    }

    static bool loginStart(Connection*);

    static bool encryptionResponse(Connection*);

public:
    static ClientLogin* protocol() {
        static ClientLogin protocol;

        return &protocol;
    }
};