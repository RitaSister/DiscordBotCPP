#pragma once

#include "../socket/Socket.h"

#include <cstddef>
#include <cstdint>

namespace NetworkSystem {

class TlsClient {
private:
    static constexpr std::size_t HASH_SIZE = 32;
    static constexpr std::size_t KEY_SIZE  = 32;
    static constexpr std::size_t IV_SIZE   = 12;
    static constexpr std::size_t TAG_SIZE  = 16;

    static constexpr std::size_t MAX_RECORD_SIZE = 18432;
    static constexpr std::size_t MAX_TRANSCRIPT  = 65536;

    Socket socket;

    // ============================================================
    // X25519
    // ============================================================

    unsigned char clientPrivateKey[32];
    unsigned char clientPublicKey[32];
    unsigned char serverPublicKey[32];
    unsigned char sharedSecret[32];

    // ============================================================
    // TLS 1.3 secrets
    // ============================================================

    unsigned char earlySecret[32];
    unsigned char handshakeSecret[32];
    unsigned char masterSecret[32];

    unsigned char clientHandshakeSecret[32];
    unsigned char serverHandshakeSecret[32];

    unsigned char clientHandshakeKey[32];
    unsigned char clientHandshakeIv[12];

    unsigned char serverHandshakeKey[32];
    unsigned char serverHandshakeIv[12];

    unsigned char clientApplicationSecret[32];
    unsigned char serverApplicationSecret[32];

    // Твои исходные имена оставляем.
    unsigned char appKey[32];
    unsigned char appIv[12];

    unsigned char serverAppKey[32];
    unsigned char serverAppIv[12];

    // ============================================================
    // Transcript
    // ============================================================

    unsigned char transcript[MAX_TRANSCRIPT];
    std::size_t transcriptLength;

    // ============================================================
    // Record sequence numbers
    // ============================================================

    std::uint64_t clientSequence;
    std::uint64_t serverSequence;

    // ============================================================
    // State
    // ============================================================

    bool handshakeCompleted;
    bool handshakeKeysReady;
    bool applicationKeysReady;

    std::uint16_t selectedCipherSuite;

    // ============================================================
    // Internal helpers
    // ============================================================

    bool generateClientKeyPair();

    bool buildClientHello(
        const char* host,
        unsigned char* output,
        std::size_t capacity,
        std::size_t& outputLength
    );

    bool appendTranscript(
        const unsigned char* data,
        std::size_t length
    );

    bool calculateTranscriptHash(
        unsigned char output[32]
    );

    bool parseServerHello(
        const unsigned char* record,
        std::size_t recordLength
    );

    bool deriveHandshakeKeys();

    bool deriveApplicationKeys();

    bool hkdfExpandLabel(
        const unsigned char* secret,
        std::size_t secretLength,
        const char* label,
        const unsigned char* context,
        std::size_t contextLength,
        unsigned char* output,
        std::size_t outputLength
    );

    bool deriveSecret(
        const unsigned char* secret,
        const char* label,
        unsigned char output[32]
    );

    void makeNonce(
        const unsigned char iv[12],
        std::uint64_t sequence,
        unsigned char nonce[12]
    );

    bool decryptRecord(
        const unsigned char* record,
        std::size_t recordLength,
        const unsigned char* key,
        const unsigned char* iv,
        std::uint64_t sequence,
        unsigned char* plaintext,
        std::size_t plaintextCapacity,
        std::size_t& plaintextLength,
        unsigned char& contentType
    );

    bool encryptRecord(
        unsigned char contentType,
        const unsigned char* plaintext,
        std::size_t plaintextLength,
        const unsigned char* key,
        const unsigned char* iv,
        std::uint64_t& sequence
    );

    bool readRecord(
        unsigned char* buffer,
        std::size_t capacity,
        std::size_t& length
    );

    bool processServerHandshake(
        const unsigned char* data,
        std::size_t length
    );

    bool processHandshakeMessage(
        const unsigned char* message,
        std::size_t length
    );

    bool processEncryptedExtensions(
        const unsigned char* body,
        std::size_t length
    );

    bool processCertificate(
        const unsigned char* body,
        std::size_t length
    );

    bool processCertificateVerify(
        const unsigned char* body,
        std::size_t length
    );

    bool processServerFinished(
        const unsigned char* body,
        std::size_t length
    );

    bool sendFinished();

    bool calculateFinishedVerifyData(
        const unsigned char finishedKey[32],
        unsigned char output[32]
    );

    // ============================================================
    // Integer helpers
    // ============================================================

    static std::uint16_t readU16(
        const unsigned char* data
    );

    static std::uint32_t readU24(
        const unsigned char* data
    );

    static std::uint32_t readU32(
        const unsigned char* data
    );

    static void writeU16(
        unsigned char* data,
        std::uint16_t value
    );

    static void writeU24(
        unsigned char* data,
        std::uint32_t value
    );

    static bool constantTimeEqual(
        const unsigned char* a,
        const unsigned char* b,
        std::size_t length
    );

public:
    TlsClient();
    ~TlsClient();

    bool connect(
        const char* host,
        unsigned short port
    );

    bool sendHttpRequest(
        const char* requestData,
        unsigned long len
    );

    long receiveResponse(
        char* buffer,
        unsigned long size
    );

    void close();
};

} // namespace NetworkSystem
