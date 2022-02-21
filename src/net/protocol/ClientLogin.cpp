#include "net/protocol/ClientLogin.h"
#include "Proxy.h"
#include "utils/Encryption.h"
#include "net/protocol/ClientPlay.h"

#include <cpprest/http_client.h>

#define ENCRYPTION_REQUEST 1

bool ClientLogin::loginStart(Connection* from) {
    ByteBuffer* buffer = from->buffer;

    //from->state = STATE_PLAYING;
    from->owner->setUsername(buffer->readString<int>(&ByteBuffer::readVarInt));
    std::cout << from->owner->getUsername() << " connecting..." << std::endl;

    if (!Proxy::instance()->isOnline()) {
        from->protocol = ClientPlay::protocol();
        JavaPacketHandler::sendToDefaultServer(from);
        return true;
    }

    ByteBuffer* encr = ByteBuffer::allocateBuffer(1100);
    int token = randomInt();

    from->owner->setToken(token);

    encr->writeByte(ENCRYPTION_REQUEST);
    encr->writeString<int>(Encryption::getHexId(), &ByteBuffer::writeVarInt);

    Encryption::addKey(encr);

    encr->writeVarInt(4);
    encr->writeInt(token);
    encr->prefixLength();

    Proxy::instance()->getJeSocket()->send(from->socket, encr);
    encr->release();
    return true;
}

bool ClientLogin::encryptionResponse(Connection* from) {
    ByteBuffer* buffer = from->buffer;
    std::string secret = buffer->readString<int>(&ByteBuffer::readVarInt);
    std::string verifyToken = buffer->readString<int>(&ByteBuffer::readVarInt);

    secret = Encryption::decodeMessage(secret);
    std::string serverHash = Encryption::genServerHash(secret);
    std::string tokenBytes = Encryption::decodeMessage(verifyToken);

    //TODO better way to do this
    buffer->setOffset(buffer->getOffset() - 4);
    buffer->writeBytes(tokenBytes.data(), tokenBytes.length());
    buffer->setSize(buffer->getSize() - 4);
    buffer->setOffset(buffer->getOffset() - 4);

    if (from->owner->getToken() != buffer->readInt()) {
        std::string msg = "Invalid Token!";
        from->owner->disconnect(msg);
        return true;
    }

    web::uri_builder query(U("https://sessionserver.mojang.com/session/minecraft/hasJoined"));

    query.append_query(U("username"), from->owner->getUsername().c_str());
    query.append_query(U("serverId"), serverHash.c_str());

    web::http::client::http_client client(query.to_uri());
    web::http::client::http_client_config cfg = client.client_config();

    cfg.set_timeout(std::chrono::seconds(5));

    pplx::task<web::http::http_response> response = client.request(web::http::methods::GET);

    response.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (response.get().status_code() != 200) {
        static std::string msg = "Failed to verify with Mojang";

        from->owner->disconnect(msg);
        return true;
    }

    web::json::value data = response.get().extract_json().get();
    std::string username = utility::conversions::to_utf8string(data.at(U("name")).as_string());
    std::string uuid = utility::conversions::to_utf8string(data.at(U("id")).as_string());

    uuid.insert(8, 1, '-');
    uuid.insert(13, 1, '-');
    uuid.insert(18, 1, '-');
    uuid.insert(23, 1, '-');

    if (username != from->owner->getUsername()) {
        static std::string msg = "Invalid Login! Please Restart.";

        from->owner->disconnect(msg);
        return true;
    }

    from->owner->setUUID(uuid);
    std::cout << "Verified " << username << " (" << uuid << ")" << std::endl;
    from->owner->handshakePacket.getAddress().append(1, '\00').append("localhost").append(1, '\00').append(uuid);

    ByteBuffer* loginPack = ByteBuffer::allocateBuffer(100);

    loginPack->writeLong(2218083794882676702);
    loginPack->writeLong(-5913475541446102882);
    loginPack->writeString<int>(username, &ByteBuffer::writeVarInt);
    loginPack->prefixLength();
    from->owner->setSecret(secret);
    from->owner->sendPacket(loginPack);

    from->protocol = ClientPlay::protocol();

    JavaPacketHandler::sendToDefaultServer(from);
    return true;
}