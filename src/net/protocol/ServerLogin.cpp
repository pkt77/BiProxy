#include "net/protocol/ServerLogin.h"

#include "net/protocol/ServerPlay.h"

bool ServerLogin::disconnect(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    std::cout << "Server is kicking " << buffer->readString(&ByteBuffer::readVarInt) << std::endl;
    return false;
}

bool ServerLogin::loginSuccess(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    std::cout << "Got Login " << buffer->readLong() << ' ' << buffer->readLong() << ' ' << buffer->readString(&ByteBuffer::readVarInt) << std::endl;
    from->protocol = ServerPlay::protocol();
    return false;
}