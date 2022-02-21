#include "net/protocol/Handshake.h"

#include <iostream>

#include "Proxy.h"
#include "net/protocol/ClientLogin.h"
#include "packets/HandshakePacket.h"
#include "net/protocol/ClientStatus.h"

bool Handshake::handshake(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    HandshakePacket hand;

    hand.read(buffer);

    std::cout << hand.getVersion() << '-' << hand.getAddress() << ':' << hand.getPort() << '+' << +hand.getState() << std::endl;

    if ((from->state = hand.getState()) == STATE_LOGIN) {
        from->owner = new Player(Proxy::instance(), from->socket);
        from->protocol = ClientLogin::protocol();
        from->owner->handshakePacket = hand;
    } else {
        from->protocol = ClientStatus::protocol();
    }

    return true;
}
