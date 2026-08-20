#pragma once

namespace CryptoSystem {

    class ChaCha20Poly1305 {
    public:
        // Шифрование и добавление тега аутентификации
        static bool encrypt(const unsigned char* key, const unsigned char* nonce,
                            const unsigned char* plaintext, unsigned long plaintextLen,
                            const unsigned char* aad, unsigned long aadLen,
                            unsigned char* ciphertext, unsigned char* tag);

        // Расшифровка и проверка тега
        static bool decrypt(const unsigned char* key, const unsigned char* nonce,
                            const unsigned char* ciphertext, unsigned long ciphertextLen,
                            const unsigned char* aad, unsigned long aadLen,
                            const unsigned char* tag, unsigned char* plaintext);
    };

} // namespace CryptoSystem
