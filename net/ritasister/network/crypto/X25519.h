#pragma once

namespace CryptoSystem {

    class X25519 {
    public:
        // Генерация общего секрета на основе нашего приватного ключа и публичного ключа сервера
        // Ключи — массивы по 32 байта
        static bool computeSharedSecret(const unsigned char* privateKey, 
                                        const unsigned char* peerPublicKey, 
                                        unsigned char* sharedSecret);

        // Генерация публичного ключа из приватного
        static void generatePublicKey(const unsigned char* privateKey, 
                                      unsigned char* publicKey);
    };

} // namespace CryptoSystem
