#include <net/JavaPacketHandler.h>

#include "Proxy.h"
#include <utils/Base64.h>

#include <iostream>
#include <utils/Encryption.h>

#include <cpprest/http_client.h>

//Connect States
#define STATE_HANDSHAKE 0
#define STATE_STATUS 1
#define STATE_LOGIN 2

//Client Packet IDs
#define HANDSHAKE 0
#define STATUS 0
#define PING 1

void JavaPacketHandler::handle(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    unsigned int startOffset;
    int length;
    unsigned int offset;
    unsigned char bytes;

    bool full = true;
    bool fromClient = from->owner == nullptr || from->socket == from->owner->getSocket();

    while (buffer->isReadable()) {
        startOffset = buffer->getOffset();

        try {
            length = buffer->readVarInt();
        } catch (char* e) {
            full = false;
            std::cout << "Header was incomplete " << e << std::endl;
        }

        offset = buffer->getOffset();

        if (!full || length > buffer->readableBytes()) {
            memcpy(buffer->getBuffer(), buffer->getBuffer() + startOffset, buffer->getSize() - startOffset);
            buffer->setSize(buffer->getSize() - startOffset);
            buffer->setOffset(0);

            if (full) {
                buffer->ensureWritable(length);
            }

            from->processing = false;
            return;
        }

        bytes = offset - startOffset;

        if (fromClient) {
            if (!handleClient(from) && from->owner != nullptr && from->owner->connectingSocket != nullptr) {
                if (!proxy->getJeSocket()->send(from->owner->connectingSocket, buffer->getBuffer() + startOffset, bytes + length)) {
                    break;
                }
            }
        } else {
            if (!handleServer(from)) {
                if (proxy->getJeSocket()->send(from->owner->getSocket(), buffer->getBuffer() + startOffset, bytes + length)) {
                    if (length > 8000) {
                        // TODO Find out why big packets like chunks need to wait, as if the buffer needs to flush... Otherwise they end up corrupted on the fromClient
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1500));
                    }
                } else {
                    break;
                }
            }
        }

        buffer->setOffset(offset + length);
    }

    buffer->setSize(0);
    buffer->setOffset(0);
    from->processing = false;
}

bool JavaPacketHandler::handleClient(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    if (from->state == STATE_HANDSHAKE) {
        if (buffer->readUnsignedByte() != HANDSHAKE) {
            //Close connection
            return true;
        }

        HandshakePacket hand;

        hand.read(buffer);

        std::cout << hand.getVersion() << '-' << hand.getAddress() << ':' << hand.getPort() << '+' << +hand.getState() << std::endl;

        if ((from->state = hand.getState()) == 2) {
            from->owner = new Player(proxy, from->socket);
            from->owner->handshakePacket = hand;
        }
        return true;
    }

    switch (buffer->readUnsignedByte()) {
        case HANDSHAKE: {
            if (from->state == STATE_STATUS) {
                std::string motdJson = proxy->getDefaultPing().toJSON();
                ByteBuffer* motd = ByteBuffer::allocateBuffer(motdJson.length() + 5);

                motd->writeByte(STATUS);
                motd->writeString<int>(motdJson, &ByteBuffer::writeVarInt);
                motd->prefixLength();

                proxy->getJeSocket()->send(from->socket, motd);
                motd->release();
                return true;
            }

            // else state should be login

            from->owner->setUsername(buffer->readString<int>(&ByteBuffer::readVarInt));
            std::cout << from->owner->getUsername() << " connecting..." << std::endl;

            ByteBuffer* encr = ByteBuffer::allocateBuffer(1100);
            int token = randomInt();

            from->owner->setToken(token);

            encr->writeByte(0x01);
            encr->writeString<int>(Encryption::getHexId(), &ByteBuffer::writeVarInt);

            Encryption::addKey(encr);

            encr->writeVarInt(4);
            encr->writeInt(token);
            encr->prefixLength();

            proxy->getJeSocket()->send(from->socket, encr);
            encr->release();
            break;
        }

        case PING: {
            if (from->state == STATE_STATUS) {
                ByteBuffer* pong = ByteBuffer::allocateBuffer(10);
                long long time = buffer->readLong();

                pong->writeVarInt(9);
                pong->writeByte(PING);
                pong->writeLong(time);

                if (from->owner == nullptr) {
                    proxy->getJeSocket()->send(from->socket, pong);
                } else {
                    proxy->getJeSocket()->send(from->owner->connectingSocket, pong);
                }

                pong->release();
            } else if (from->state == STATE_LOGIN) {
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

                from->owner->setSecret(secret);

                if (from->owner->getToken() != buffer->readInt()) {
                    std::string msg = "Invalid Token!";
                    from->owner->disconnect(msg);
                    break;
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

                if (response.get().status_code() == 200) {
                    web::json::value data = response.get().extract_json().get();
                    std::string username = utility::conversions::to_utf8string(data.at(U("name")).as_string());
                    std::string uuid = utility::conversions::to_utf8string(data.at(U("id")).as_string());
                    //TODO check if logged in

                    uuid.insert(8, 1, '-');
                    uuid.insert(13, 1, '-');
                    uuid.insert(18, 1, '-');
                    uuid.insert(23, 1, '-');

                    from->owner->setUUID(uuid);
                    from->owner->handshakePacket.getAddress().append(1, '\00').append("localhost").append(1, '\00').append(uuid);

                    std::string server = "hub";
                    Server* hub = proxy->getServer(server);

                    if (hub == nullptr) {
                        std::string msg = "No default server";
                        from->owner->disconnect(msg);
                    } else if (!from->owner->connect(hub)) {
                        std::string msg = "Failed to connect to default server";
                        from->owner->disconnect(msg);
                    }
                } else {
                    std::string msg = "Failed to verify with Mojang";
                    from->owner->disconnect(msg);
                }
            }
            break;
        }

/*        case 3: {
            CompressionPacket comp;

            comp.read(buffer);

            std::cout << comp.getThreshold() << std::endl;
            break;
        }*/

        default:
            return false;
    }
    return true;
}

bool JavaPacketHandler::handleServer(Connection* from) const {
    ByteBuffer* buffer = from->buffer;

    switch (buffer->readUnsignedByte()) {
        /*     case 0:
                 std::cout << buffer->readString() << std::endl;
                 break;*/

        case 0x02:
            //std::cout << "Got Login" << std::endl;
            break;

        case 0x24: {
            // std::ofstream out("data.nbt", std::ios::binary);

            //out.write(buffer->getBuffer() + buffer->getOffset(), length - 1);
            break;
        }

        case 0x17: {
            std::cout << buffer->readString<int>(&ByteBuffer::readVarInt) << std::endl;
            break;
        }

        case 0x19:
            std::cout << buffer->readString<int>(&ByteBuffer::readVarInt) << std::endl;
            break;

        default:
            return false;
    }
    return false; // Temporary
}

void JavaPacketHandler::disconnect(Connection* from) const {
    if (from->owner == nullptr || from->socket == from->owner->getSocket()) {
        if (from->owner != nullptr) {
            std::cout << from->owner->getUsername() << " Disconnected" << std::endl;
        }
    } else {
        std::string msg = "Server disconnected";
        // from->owner->disconnect(msg);
        std::cout << msg << std::endl;
    }
}