#pragma once

namespace CryptoSystem {

    // Размер хэша SHA-256 в байтах
    constexpr int SHA256_DIGEST_SIZE = 32;

    class CryptoEngine {
    public:
        // Хэш SHA-256
        static void sha256(const unsigned char* data, unsigned long len, unsigned char* output);

        // HMAC-SHA256
        static void hmacSha256(const unsigned char* key, unsigned long keyLen,
                               const unsigned char* data, unsigned long dataLen,
                               unsigned char* output);

        // HKDF-Extract (RFC 5869)
        static void hkdfExtract(const unsigned char* salt, unsigned long saltLen,
                                const unsigned char* ikm, unsigned long ikmLen,
                                unsigned char* prk);

        // HKDF-Expand (RFC 5869)
        static void hkdfExpand(const unsigned char* prk, unsigned long prkLen,
                               const unsigned char* info, unsigned long infoLen,
                               unsigned char* okm, unsigned long okmLen);
    };

} // namespace CryptoSystem
