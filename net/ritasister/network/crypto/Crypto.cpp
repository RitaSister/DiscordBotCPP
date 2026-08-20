#include "Crypto.h"

namespace CryptoSystem {
    // Константы SHA-256
    static const unsigned long K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    inline unsigned long rotr(unsigned long val, unsigned long shift) {
        return (val >> shift) | (val << (32 - shift));
    }

    void CryptoEngine::sha256(const unsigned char *data, unsigned long len, unsigned char *output) {
        unsigned long h[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        // Подготовка паддинга (до 64-байтовых блоков)
        unsigned long totalLen = len + 1 + 8;
        unsigned long blocksCount = ((totalLen + 63) / 64);
        unsigned long paddedLen = blocksCount * 64;

        // Используем только статический буфер под паддинг (-nostdlib)
        static unsigned char staticBuf[4096];
        if (paddedLen > 4096) {
            return; // Слишком большой пакет
        }

        for (unsigned long i = 0; i < paddedLen; ++i) staticBuf[i] = 0;
        for (unsigned long i = 0; i < len; ++i) staticBuf[i] = data[i];

        staticBuf[len] = 0x80;
        unsigned long long bitLen = static_cast<unsigned long long>(len) * 8;
        for (int i = 0; i < 8; ++i) {
            staticBuf[paddedLen - 1 - i] = static_cast<unsigned char>(bitLen & 0xFF);
            bitLen >>= 8;
        }

        // Цикл обработки блоков
        for (unsigned long b = 0; b < blocksCount; ++b) {
            unsigned long w[64];
            const unsigned char *chunk = staticBuf + (b * 64);

            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<unsigned long>(chunk[i * 4]) << 24) |
                       (static_cast<unsigned long>(chunk[i * 4 + 1]) << 16) |
                       (static_cast<unsigned long>(chunk[i * 4 + 2]) << 8) |
                       (static_cast<unsigned long>(chunk[i * 4 + 3]));
            }

            for (int i = 16; i < 64; ++i) {
                unsigned long s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                unsigned long s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            unsigned long a = h[0], bb = h[1], c = h[2], d = h[3];
            unsigned long e = h[4], f = h[5], g = h[6], hh = h[7];

            for (int i = 0; i < 64; ++i) {
                unsigned long S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                unsigned long ch = (e & f) ^ (~e & g);
                unsigned long temp1 = hh + S1 + ch + K[i] + w[i];
                unsigned long S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                unsigned long maj = (a & bb) ^ (a & c) ^ (bb & c);
                unsigned long temp2 = S0 + maj;

                hh = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = bb;
                bb = a;
                a = temp1 + temp2;
            }

            h[0] += a;
            h[1] += bb;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }

        for (int i = 0; i < 8; ++i) {
            output[i * 4] = static_cast<unsigned char>(h[i] >> 24);
            output[i * 4 + 1] = static_cast<unsigned char>(h[i] >> 16);
            output[i * 4 + 2] = static_cast<unsigned char>(h[i] >> 8);
            output[i * 4 + 3] = static_cast<unsigned char>(h[i]);
        }
    }

    void CryptoEngine::hmacSha256(const unsigned char *key, unsigned long keyLen,
                                  const unsigned char *data, unsigned long dataLen,
                                  unsigned char *output) {
        unsigned char k0[64] = {0};
        if (keyLen > 64) {
            sha256(key, keyLen, k0);
        } else {
            for (unsigned long i = 0; i < keyLen; ++i) k0[i] = key[i];
        }

        unsigned char ipad[64], opad[64];
        for (int i = 0; i < 64; ++i) {
            ipad[i] = k0[i] ^ 0x36;
            opad[i] = k0[i] ^ 0x5c;
        }

        // inner hash
        // (ipad || data)
        static unsigned char innerBuf[4096];
        unsigned long innerLen = 64 + dataLen;
        for (int i = 0; i < 64; ++i) innerBuf[i] = ipad[i];
        for (unsigned long i = 0; i < dataLen; ++i) innerBuf[64 + i] = data[i];

        unsigned char innerHash[32];
        sha256(innerBuf, innerLen, innerHash);

        // outer hash
        // (opad || innerHash)
        unsigned char outerBuf[64 + 32];
        for (int i = 0; i < 64; ++i) outerBuf[i] = opad[i];
        for (int i = 0; i < 32; ++i) outerBuf[64 + i] = innerHash[i];

        sha256(outerBuf, 96, output);
    }

    void CryptoEngine::hkdfExtract(const unsigned char *salt, unsigned long saltLen,
                                   const unsigned char *ikm, unsigned long ikmLen,
                                   unsigned char *prk) {
        // Если salt пустой, берем 32 нулевых байта
        unsigned char defaultSalt[32] = {0};
        if (salt == nullptr || saltLen == 0) {
            salt = defaultSalt;
            saltLen = 32;
        }
        hmacSha256(salt, saltLen, ikm, ikmLen, prk);
    }

    void CryptoEngine::hkdfExpand(const unsigned char *prk, unsigned long prkLen,
                                  const unsigned char *info, unsigned long infoLen,
                                  unsigned char *okm, unsigned long okmLen) {
        // Упрощенный вариант HKDF-Expand для генерации ключей TLS 1.3 (okmLen <= 32)
        unsigned char t[32];
        unsigned char hmacInput[256];

        unsigned long inLen = infoLen + 1;
        for (unsigned long i = 0; i < infoLen; ++i) {
            hmacInput[i] = info[i];
        }
        hmacInput[infoLen] = 0x01; // Block counter T(1)

        hmacSha256(prk, prkLen, hmacInput, inLen, t);

        for (unsigned long i = 0; i < okmLen && i < 32; ++i) {
            okm[i] = t[i];
        }
    }
} // namespace CryptoSystem
