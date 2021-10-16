#include "utils/Encryption.h"

#include <utils/Base64.h>
#include <algorithm>

#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <iostream>

char hexId[17];

RSA* serverKey;

std::vector<BYTE> raw;

void Encryption::init(long long id) {
    serverKey = RSA_generate_key(1024, 3, NULL, NULL);

    BIO* bio = BIO_new(BIO_s_mem());

    PEM_write_bio_RSA_PUBKEY(bio, serverKey);

    int length = BIO_pending(bio);
    ByteBuffer* pemBuffer = ByteBuffer::allocateBuffer(length);

    BIO_read(bio, pemBuffer->getBuffer(), length);

    std::string based(pemBuffer->getBuffer() + 27, length - 28 - 25);
    based.erase(std::remove(based.begin(), based.end(), '\n'), based.end());

    raw = base64_decode(based);
    pemBuffer->release();

    _itoa_s(id, hexId, 16);
}

char* Encryption::getHexId() {
    return hexId;
}

void Encryption::addKey(ByteBuffer* buffer) {
    buffer->writeVarInt(raw.size());
    buffer->writeBytes(raw.data(), raw.size());
}

std::string Encryption::genServerHash(const std::string& secret) {
    SHA_CTX hash;
    unsigned char hashBuffer[20];
    std::string hexHash = std::string();

    hexHash.reserve(21);

    SHA1_Init(&hash);
    SHA1_Update(&hash, hexId, std::strlen(hexId));
    SHA1_Update(&hash, secret.data(), secret.length());
    SHA1_Update(&hash, raw.data(), raw.size());
    SHA1_Final(hashBuffer, &hash);

    if (hashBuffer[0] & 0x80) { //Is Negative
        unsigned char carry = 1;

        hexHash += '-';

        for (int i = 19; i >= 0; i--) {
            unsigned short twocomp = (unsigned char) ~hashBuffer[i];
            twocomp += carry;

            if (twocomp & 0xff00) {
                twocomp = twocomp & 0xff;
            } else {
                carry = 0;
            }

            hashBuffer[i] = (uint8_t) twocomp;
        }
    }

    static const char hex[] = "0123456789abcdef";

    for (int i = 0; i < 20; i++) {
        if (i || hashBuffer[i] >> 4) {
            hexHash += hex[(hashBuffer[i] >> 4)];
        }

        if (i || hashBuffer[i] >> 4 || hashBuffer[i] & 0xf) {
            hexHash += hex[(hashBuffer[i] & 0xf)];
        }
    }

    return hexHash;
}

std::string Encryption::decodeMessage(const std::string& secret) {
    ByteBuffer* out = ByteBuffer::allocateBuffer(secret.length());
    int length = RSA_private_decrypt(secret.length(), (unsigned char*) secret.data(), (unsigned char*) out->getBuffer(), serverKey, RSA_PKCS1_PADDING);

    if (length == -1) {
        return NULL;
    }

    std::string message(out->getBuffer(), length);

    out->release();

    return message;
}

void Encryption::encode(std::string secret, char* buffer, int length) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    EVP_CipherInit(ctx, EVP_aes_128_cfb8(), (unsigned char*) secret.data(), (unsigned char*) secret.data(), true);
    EVP_CipherUpdate(ctx, (unsigned char*) buffer, &length, (unsigned char*) buffer, length);
    EVP_CIPHER_CTX_free(ctx);
}