#pragma once

#include "data/Player.h"

struct Connection;

typedef bool (* PacketHandler)(Connection*);

#define STATE_HANDSHAKE 0
#define STATE_STATUS 1
#define STATE_LOGIN 2
#define STATE_PLAYING 3

class Protocol {
private:
    unsigned char size;
    PacketHandler* handlers;

protected:
    Protocol(unsigned char size) : size(size) {
        handlers = size == 0 ? nullptr : new PacketHandler[size]{};
    }

public:
    void setHandler(unsigned char id, PacketHandler handler) {
        if (id < size) {
            handlers[id] = handler;
        }
    }

    virtual PacketHandler getHandler(unsigned char id) {
        return id < size ? handlers[id] : nullptr;
    }
};

struct Connection {
    void* socket;
    Protocol* protocol;
    ByteBuffer* buffer;
    bool processing;
    unsigned char state;
    Player* owner;
};