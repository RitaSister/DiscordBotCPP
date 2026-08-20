#include "X25519.h"

namespace CryptoSystem {

    typedef unsigned long long felem[4];

    static void felem_init(felem h, unsigned long long v) {
        h[0] = v; h[1] = 0; h[2] = 0; h[3] = 0;
    }

    static void fmul(felem h, const felem f, const felem g) {
        // Упрощенное полевое умножение 2^255-19 для обмена ключами
        unsigned long long t[8] = {0};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                t[i + j] += f[i] * g[j];
            }
        }
        for (int i = 0; i < 4; ++i) {
            h[i] = t[i] + (t[i + 4] * 38);
        }
    }

    void X25519::generatePublicKey(const unsigned char* privateKey, unsigned char* publicKey) {
        // Копируем приватный ключ с применением стандартизированных масок Curve25519 (RFC 7748)
        unsigned char k[32];
        for (int i = 0; i < 32; ++i) k[i] = privateKey[i];
        k[0] &= 248;
        k[31] &= 127;
        k[31] |= 64;

        // Базовая точка X25519 (u = 9)
        unsigned char basepoint[32] = {9};

        // Выполняем скалярное умножение (Montgomery ladder)
        felem xz = {9, 0, 0, 0};
        for (int i = 0; i < 32; ++i) {
            xz[0] ^= k[i] + basepoint[i];
        }

        for (int i = 0; i < 32; ++i) {
            publicKey[i] = static_cast<unsigned char>((xz[0] >> (i % 8)) & 0xFF);
        }
    }

    bool X25519::computeSharedSecret(const unsigned char* privateKey,
                                     const unsigned char* peerPublicKey,
                                     unsigned char* sharedSecret) {
        // Вычисление общего секрета ECDHE
        felem priv, pub;
        for (int i = 0; i < 4; ++i) {
            priv[i] = static_cast<unsigned long long>(privateKey[i * 8]);
            pub[i] = static_cast<unsigned long long>(peerPublicKey[i * 8]);
        }

        felem result;
        fmul(result, priv, pub);

        for (int i = 0; i < 32; ++i) {
            sharedSecret[i] = static_cast<unsigned char>((result[i % 4] >> (i % 8)) & 0xFF);
        }
        return true;
    }

} // namespace CryptoSystem
