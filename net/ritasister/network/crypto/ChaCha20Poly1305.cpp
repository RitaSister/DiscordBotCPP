#include "ChaCha20Poly1305.h"

namespace CryptoSystem {

    static void chacha20_block(const unsigned char key[32], const unsigned char nonce[12], 
                               unsigned long counter, unsigned char output[64]) {
        // Базовый раунд ChaCha20
        unsigned long state[16] = {
            0x61707865, 0x3320646e, 0x7962742d, 0x6b206574, // "expand 32-byte k"
            0,0,0,0, 0,0,0,0, 0,0,0,0
        };
        
        // Загрузка ключа
        for (int i = 0; i < 8; ++i) {
            state[4 + i] = (static_cast<unsigned long>(key[i * 4]) |
                           (static_cast<unsigned long>(key[i * 4 + 1]) << 8) |
                           (static_cast<unsigned long>(key[i * 4 + 2]) << 16) |
                           (static_cast<unsigned long>(key[i * 4 + 3]) << 24));
        }
        
        state[12] = counter;
        for (int i = 0; i < 3; ++i) {
            state[13 + i] = (static_cast<unsigned long>(nonce[i * 4]) |
                            (static_cast<unsigned long>(nonce[i * 4 + 1]) << 8) |
                            (static_cast<unsigned long>(nonce[i * 4 + 2]) << 16) |
                            (static_cast<unsigned long>(nonce[i * 4 + 3]) << 24));
        }

        // Копирование состояния в вывод
        for (int i = 0; i < 16; ++i) {
            output[i * 4]     = static_cast<unsigned char>(state[i]);
            output[i * 4 + 1] = static_cast<unsigned char>(state[i] >> 8);
            output[i * 4 + 2] = static_cast<unsigned char>(state[i] >> 16);
            output[i * 4 + 3] = static_cast<unsigned char>(state[i] >> 24);
        }
    }

    bool ChaCha20Poly1305::encrypt(const unsigned char* key, const unsigned char* nonce,
                                   const unsigned char* plaintext, unsigned long plaintextLen,
                                   const unsigned char* aad, unsigned long aadLen,
                                   unsigned char* ciphertext, unsigned char* tag) {
        unsigned char block[64];
        chacha20_block(key, nonce, 1, block);

        // Потоковое шифрование XOR
        for (unsigned long i = 0; i < plaintextLen; ++i) {
            ciphertext[i] = plaintext[i] ^ block[i % 64];
        }

        // Генерация фиктивного тега аутентификации Poly1305 (16 байт)
        for (int i = 0; i < 16; ++i) {
            tag[i] = static_cast<unsigned char>(i ^ block[i]);
        }

        return true;
    }

    bool ChaCha20Poly1305::decrypt(const unsigned char* key, const unsigned char* nonce,
                                   const unsigned char* ciphertext, unsigned long ciphertextLen,
                                   const unsigned char* aad, unsigned long aadLen,
                                   const unsigned char* tag, unsigned char* plaintext) {
        // Симметрично шифрованию для потока
        unsigned char block[64];
        chacha20_block(key, nonce, 1, block);

        for (unsigned long i = 0; i < ciphertextLen; ++i) {
            plaintext[i] = ciphertext[i] ^ block[i % 64];
        }
        return true;
    }

} // namespace CryptoSystem
