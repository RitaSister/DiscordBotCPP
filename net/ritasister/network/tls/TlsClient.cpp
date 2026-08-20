#include "TlsClient.h"

#include "../../network/crypto/Crypto.h"
#include "../../network/crypto/X25519.h"
#include "../../network/crypto/ChaCha20Poly1305.h"
#include "../../memory/CoreMemory.h"

namespace NetworkSystem {
    namespace {
        constexpr unsigned char TLS_HANDSHAKE = 22;
        constexpr unsigned char TLS_ALERT = 21;
        constexpr unsigned char TLS_APPLICATION_DATA = 23;

        constexpr unsigned char HS_CLIENT_HELLO = 1;
        constexpr unsigned char HS_SERVER_HELLO = 2;
        constexpr unsigned char HS_ENCRYPTED_EXTENSIONS = 8;
        constexpr unsigned char HS_CERTIFICATE = 11;
        constexpr unsigned char HS_CERTIFICATE_VERIFY = 15;
        constexpr unsigned char HS_FINISHED = 20;

        constexpr std::uint16_t TLS13 = 0x0304;
        constexpr std::uint16_t X25519 = 0x001d;

        constexpr std::uint16_t TLS_AES_128_GCM_SHA256 = 0x1301;
        constexpr std::uint16_t TLS_CHACHA20_POLY1305_SHA256 = 0x1303;
    }

    /* ================================================================
     * Constructor
     * ================================================================ */

    TlsClient::TlsClient()
        : transcriptLength(0),
          clientSequence(0),
          serverSequence(0),
          handshakeCompleted(false),
          handshakeKeysReady(false),
          applicationKeysReady(false),
          selectedCipherSuite(0) {

        // Используем memcpy/memset через глобальные функции из CoreMemory
        memset(clientPrivateKey, 0, sizeof(clientPrivateKey));
        memset(clientPublicKey, 0, sizeof(clientPublicKey));
        memset(serverPublicKey, 0, sizeof(serverPublicKey));
        memset(sharedSecret, 0, sizeof(sharedSecret));

        memset(earlySecret, 0, sizeof(earlySecret));
        memset(handshakeSecret, 0, sizeof(handshakeSecret));
        memset(masterSecret, 0, sizeof(masterSecret));

        memset(clientHandshakeSecret, 0, sizeof(clientHandshakeSecret));
        memset(serverHandshakeSecret, 0, sizeof(serverHandshakeSecret));

        memset(clientHandshakeKey, 0, sizeof(clientHandshakeKey));
        memset(clientHandshakeIv, 0, sizeof(clientHandshakeIv));
        memset(serverHandshakeKey, 0, sizeof(serverHandshakeKey));
        memset(serverHandshakeIv, 0, sizeof(serverHandshakeIv));

        memset(clientApplicationSecret, 0, sizeof(clientApplicationSecret));
        memset(serverApplicationSecret, 0, sizeof(serverApplicationSecret));

        memset(appKey, 0, sizeof(appKey));
        memset(appIv, 0, sizeof(appIv));
        memset(serverAppKey, 0, sizeof(serverAppKey));
        memset(serverAppIv, 0, sizeof(serverAppIv));

        generateClientKeyPair();
    }

    TlsClient::~TlsClient() {
        close();
    }

    /* ================================================================
     * X25519
     * ================================================================ */

    bool TlsClient::generateClientKeyPair() {
        for (int i = 0; i < 32; ++i) {
            clientPrivateKey[i] =
                    static_cast<unsigned char>(
                        (i * 17 + 5) & 0xff
                    );
        }

        CryptoSystem::X25519::generatePublicKey(
            clientPrivateKey,
            clientPublicKey
        );

        return true;
    }

    /* ================================================================
     * Integer helpers
     * ================================================================ */

    std::uint16_t TlsClient::readU16(
        const unsigned char *p
    ) {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(p[0]) << 8) |
            p[1]
        );
    }

    std::uint32_t TlsClient::readU24(
        const unsigned char *p
    ) {
        return
                (static_cast<std::uint32_t>(p[0]) << 16) |
                (static_cast<std::uint32_t>(p[1]) << 8) |
                p[2];
    }

    std::uint32_t TlsClient::readU32(
        const unsigned char *p
    ) {
        return
                (static_cast<std::uint32_t>(p[0]) << 24) |
                (static_cast<std::uint32_t>(p[1]) << 16) |
                (static_cast<std::uint32_t>(p[2]) << 8) |
                p[3];
    }

    void TlsClient::writeU16(
        unsigned char *p,
        std::uint16_t value
    ) {
        p[0] = static_cast<unsigned char>(value >> 8);
        p[1] = static_cast<unsigned char>(value);
    }

    void TlsClient::writeU24(
        unsigned char *p,
        std::uint32_t value
    ) {
        p[0] = static_cast<unsigned char>(value >> 16);
        p[1] = static_cast<unsigned char>(value >> 8);
        p[2] = static_cast<unsigned char>(value);
    }

    bool TlsClient::constantTimeEqual(
        const unsigned char *a,
        const unsigned char *b,
        std::size_t length
    ) {
        unsigned char diff = 0;

        for (std::size_t i = 0; i < length; ++i)
            diff |= static_cast<unsigned char>(a[i] ^ b[i]);

        return diff == 0;
    }

    /* ================================================================
     * Transcript
     * ================================================================ */

    bool TlsClient::appendTranscript(
        const unsigned char *data,
        std::size_t length
    ) {
        if (transcriptLength + length > MAX_TRANSCRIPT)
            return false;

        memcpy(
            transcript + transcriptLength,
            data,
            length
        );

        transcriptLength += length;

        return true;
    }

    bool TlsClient::calculateTranscriptHash(
        unsigned char output[32]
    ) {
        CryptoSystem::CryptoEngine::sha256(
            transcript,
            transcriptLength,
            output
        );

        return true;
    }

    /* ================================================================
     * HKDF-Expand-Label
     * ================================================================ */

    bool TlsClient::hkdfExpandLabel(
        const unsigned char *secret,
        std::size_t secretLength,
        const char *label,
        const unsigned char *context,
        std::size_t contextLength,
        unsigned char *output,
        std::size_t outputLength
    ) {
        unsigned char info[512];

        const char prefix[] = "tls13 ";

        const std::size_t labelLength = strlen(label);
        const std::size_t fullLabelLength =
                (sizeof(prefix) - 1) + labelLength;

        if (fullLabelLength > 255)
            return false;

        if (contextLength > 255)
            return false;

        if (outputLength > 65535)
            return false;

        std::size_t p = 0;

        info[p++] = static_cast<unsigned char>(
            (outputLength >> 8) & 0xFF
        );

        info[p++] = static_cast<unsigned char>(
            outputLength & 0xFF
        );

        info[p++] = static_cast<unsigned char>(
            fullLabelLength
        );

        memcpy(
            info + p,
            prefix,
            sizeof(prefix) - 1
        );

        p += sizeof(prefix) - 1;

        memcpy(
            info + p,
            label,
            labelLength
        );

        p += labelLength;

        info[p++] = static_cast<unsigned char>(
            contextLength
        );

        if (contextLength != 0) {
            if (context == nullptr)
                return false;

            memcpy(
                info + p,
                context,
                contextLength
            );

            p += contextLength;
        }

        CryptoSystem::CryptoEngine::hkdfExpand(
            secret,
            secretLength,
            info,
            p,
            output,
            outputLength
        );

        return true;
    }

    /* ================================================================
     * Derive-Secret
     * ================================================================ */

    bool TlsClient::deriveSecret(
        const unsigned char *secret,
        const char *label,
        unsigned char output[32]
    ) {
        unsigned char hash[32];

        if (!calculateTranscriptHash(hash))
            return false;

        return hkdfExpandLabel(
            secret,
            32,
            label,
            hash,
            32,
            output,
            32
        );
    }

    /* ================================================================
     * ClientHello
     * ================================================================ */

    bool TlsClient::buildClientHello(
        const char *host,
        unsigned char *out,
        std::size_t capacity,
        std::size_t &outLength
    ) {
        if (capacity < 1024)
            return false;

        if (host == nullptr)
            return false;

        std::size_t p = 0;

        out[p++] = TLS_HANDSHAKE;
        out[p++] = 0x03;
        out[p++] = 0x03;

        const std::size_t recordLengthPos = p;
        p += 2;

        out[p++] = HS_CLIENT_HELLO;

        const std::size_t handshakeLengthPos = p;
        p += 3;

        out[p++] = 0x03;
        out[p++] = 0x03;

        for (int i = 0; i < 32; ++i) {
            out[p++] =
                    static_cast<unsigned char>(
                        ((i * 31) ^ 0xAC) & 0xff
                    );
        }

        out[p++] = 0;

        writeU16(out + p, 4);
        p += 2;

        writeU16(
            out + p,
            TLS_CHACHA20_POLY1305_SHA256
        );
        p += 2;

        writeU16(
            out + p,
            TLS_AES_128_GCM_SHA256
        );
        p += 2;

        out[p++] = 1;
        out[p++] = 0;

        const std::size_t extensionsLengthPos = p;
        p += 2;

        const std::size_t hostLength = strlen(host);

        if (hostLength > 253)
            return false;

        writeU16(out + p, 0x0000);
        p += 2;

        writeU16(
            out + p,
            static_cast<std::uint16_t>(
                hostLength + 5
            )
        );
        p += 2;

        writeU16(
            out + p,
            static_cast<std::uint16_t>(
                hostLength + 3
            )
        );
        p += 2;

        out[p++] = 0;

        writeU16(
            out + p,
            static_cast<std::uint16_t>(hostLength)
        );
        p += 2;

        memcpy(
            out + p,
            host,
            hostLength
        );

        p += hostLength;

        writeU16(out + p, 0x002b);
        p += 2;

        writeU16(out + p, 3);
        p += 2;

        out[p++] = 2;

        writeU16(out + p, TLS13);
        p += 2;

        writeU16(out + p, 0x000a);
        p += 2;

        writeU16(out + p, 4);
        p += 2;

        writeU16(out + p, 2);
        p += 2;

        writeU16(out + p, X25519);
        p += 2;

        writeU16(out + p, 0x000d);
        p += 2;

        writeU16(out + p, 6);
        p += 2;

        writeU16(out + p, 4);
        p += 2;

        writeU16(out + p, 0x0403);
        p += 2;

        writeU16(out + p, 0x0804);
        p += 2;

        writeU16(out + p, 0x0033);
        p += 2;

        writeU16(out + p, 38);
        p += 2;

        writeU16(out + p, X25519);
        p += 2;

        writeU16(out + p, 32);
        p += 2;

        memcpy(
            out + p,
            clientPublicKey,
            32
        );

        p += 32;

        writeU16(
            out + extensionsLengthPos,
            static_cast<std::uint16_t>(
                p - extensionsLengthPos - 2
            )
        );

        writeU24(
            out + handshakeLengthPos,
            static_cast<std::uint32_t>(
                p - handshakeLengthPos - 3
            )
        );

        writeU16(
            out + recordLengthPos,
            static_cast<std::uint16_t>(
                p - 5
            )
        );

        outLength = p;

        return true;
    }

    /* ================================================================
     * ServerHello parser
     * ================================================================ */

    bool TlsClient::parseServerHello(
        const unsigned char *record,
        std::size_t recordLength
    ) {
        if (recordLength < 5)
            return false;

        if (record[0] != TLS_HANDSHAKE)
            return false;

        const std::uint16_t tlsRecordLength =
                readU16(record + 3);

        if (
            static_cast<std::size_t>(tlsRecordLength) + 5 >
            recordLength
        )
            return false;

        std::size_t p = 5;

        if (record[p++] != HS_SERVER_HELLO)
            return false;

        const std::uint32_t helloLength =
                readU24(record + p);

        p += 3;

        if (
            static_cast<std::size_t>(helloLength) >
            recordLength - p
        )
            return false;

        p += 2;
        p += 32;

        if (p >= recordLength)
            return false;

        const unsigned char sessionIdLength =
                record[p++];

        if (p + sessionIdLength > recordLength)
            return false;

        p += sessionIdLength;

        if (p + 2 > recordLength)
            return false;

        selectedCipherSuite =
                readU16(record + p);

        p += 2;

        if (p >= recordLength)
            return false;

        if (record[p++] != 0)
            return false;

        if (p + 2 > recordLength)
            return false;

        const std::uint16_t extensionsLength =
                readU16(record + p);

        p += 2;

        const std::size_t extensionsEnd =
                p + extensionsLength;

        if (extensionsEnd > recordLength)
            return false;

        bool foundVersion = false;
        bool foundKeyShare = false;

        while (p + 4 <= extensionsEnd) {
            const std::uint16_t type =
                    readU16(record + p);

            const std::uint16_t length =
                    readU16(record + p + 2);

            p += 4;

            if (p + length > extensionsEnd)
                return false;

            if (type == 0x002b) {
                if (length != 2)
                    return false;

                if (readU16(record + p) != TLS13)
                    return false;

                foundVersion = true;
            }
            else if (type == 0x0033) {
                if (length != 36)
                    return false;

                const std::uint16_t group =
                        readU16(record + p);

                const std::uint16_t keyLength =
                        readU16(record + p + 2);

                if (group != X25519)
                    return false;

                if (keyLength != 32)
                    return false;

                memcpy(
                    serverPublicKey,
                    record + p + 4,
                    32
                );

                foundKeyShare = true;
            }

            p += length;
        }

        if (!foundVersion || !foundKeyShare)
            return false;

        if (
            selectedCipherSuite !=
            TLS_CHACHA20_POLY1305_SHA256 &&
            selectedCipherSuite !=
            TLS_AES_128_GCM_SHA256
        )
            return false;

        return true;
    }

    /* ================================================================
     * Handshake keys
     * ================================================================ */

    bool TlsClient::deriveHandshakeKeys() {
        unsigned char zeros[32] = {0};
        unsigned char emptyHash[32];

        CryptoSystem::CryptoEngine::sha256(
            nullptr,
            0,
            emptyHash
        );

        CryptoSystem::CryptoEngine::hkdfExtract(
            zeros,
            32,
            nullptr,
            0,
            earlySecret
        );

        unsigned char derivedSecret[32];

        if (!hkdfExpandLabel(
            earlySecret,
            32,
            "derived",
            emptyHash,
            32,
            derivedSecret,
            32
        )) {
            return false;
        }

        CryptoSystem::CryptoEngine::hkdfExtract(
            derivedSecret,
            32,
            sharedSecret,
            32,
            handshakeSecret
        );

        unsigned char transcriptHash[32];

        CryptoSystem::CryptoEngine::sha256(
            transcript,
            static_cast<unsigned long>(transcriptLength),
            transcriptHash
        );

        if (!hkdfExpandLabel(
            handshakeSecret,
            32,
            "c hs traffic",
            transcriptHash,
            32,
            clientHandshakeSecret,
            32
        )) {
            return false;
        }

        if (!hkdfExpandLabel(
            handshakeSecret,
            32,
            "s hs traffic",
            transcriptHash,
            32,
            serverHandshakeSecret,
            32
        )) {
            return false;
        }

        if (!hkdfExpandLabel(
            clientHandshakeSecret,
            32,
            "key",
            nullptr,
            0,
            clientHandshakeKey,
            32
        )) {
            return false;
        }

        if (!hkdfExpandLabel(
            clientHandshakeSecret,
            32,
            "iv",
            nullptr,
            0,
            clientHandshakeIv,
            12
        )) {
            return false;
        }

        if (!hkdfExpandLabel(
            serverHandshakeSecret,
            32,
            "key",
            nullptr,
            0,
            serverHandshakeKey,
            32
        )) {
            return false;
        }

        if (!hkdfExpandLabel(
            serverHandshakeSecret,
            32,
            "iv",
            nullptr,
            0,
            serverHandshakeIv,
            12
        )) {
            return false;
        }

        clientSequence = 0;
        serverSequence = 0;
        handshakeKeysReady = true;

        return true;
    }

    /* ================================================================
     * Nonce
     * ================================================================ */

    void TlsClient::makeNonce(
        const unsigned char iv[12],
        std::uint64_t sequence,
        unsigned char nonce[12]
    ) {
        memcpy(nonce, iv, 12);

        for (int i = 0; i < 8; ++i) {
            nonce[11 - i] ^=
                    static_cast<unsigned char>(
                        sequence >> (i * 8)
                    );
        }
    }

    /* ================================================================
     * TLS record decryption
     * ================================================================ */

    bool TlsClient::decryptRecord(
        const unsigned char *record,
        std::size_t recordLength,
        const unsigned char *key,
        const unsigned char *iv,
        std::uint64_t sequence,
        unsigned char *plaintext,
        std::size_t plaintextCapacity,
        std::size_t &plaintextLength,
        unsigned char &contentType
    ) {
        if (recordLength < 5 + TAG_SIZE)
            return false;

        if (record[0] != TLS_APPLICATION_DATA &&
            record[0] != TLS_HANDSHAKE)
            return false;

        const std::uint16_t encryptedLength =
                readU16(record + 3);

        if (
            static_cast<std::size_t>(encryptedLength) + 5 >
            recordLength
        )
            return false;

        if (encryptedLength < TAG_SIZE)
            return false;

        const std::size_t ciphertextLength =
                encryptedLength - TAG_SIZE;

        if (ciphertextLength > plaintextCapacity)
            return false;

        const unsigned char *ciphertext = record + 5;
        const unsigned char *tag = ciphertext + ciphertextLength;

        unsigned char nonce[12];
        makeNonce(iv, sequence, nonce);

        if (!CryptoSystem::ChaCha20Poly1305::decrypt(
            key,
            nonce,
            ciphertext,
            ciphertextLength,
            record,
            5,
            tag,
            plaintext
        )) {
            return false;
        }
        plaintextLength = ciphertextLength;

        while (
            plaintextLength > 0 &&
            plaintext[plaintextLength - 1] == 0
        ) {
            --plaintextLength;
        }

        if (plaintextLength == 0)
            return false;

        contentType = plaintext[plaintextLength - 1];
        --plaintextLength;

        return true;
    }

    /* ================================================================
     * TLS record encryption
     * ================================================================ */

    bool TlsClient::encryptRecord(
        unsigned char contentType,
        const unsigned char *plaintext,
        std::size_t plaintextLength,
        const unsigned char *key,
        const unsigned char *iv,
        std::uint64_t &sequence
    ) {
        if (plaintextLength > 16384)
            return false;

        unsigned char inner[16385];
        memcpy(inner, plaintext, plaintextLength);

        inner[plaintextLength] = contentType;
        const std::size_t innerLength = plaintextLength + 1;

        unsigned char nonce[12];
        makeNonce(iv, sequence, nonce);

        unsigned char ciphertext[16385];
        unsigned char tag[16];
        unsigned char header[5];

        header[0] = TLS_APPLICATION_DATA;
        header[1] = 0x03;
        header[2] = 0x03;

        writeU16(
            header + 3,
            static_cast<std::uint16_t>(
                innerLength + TAG_SIZE
            )
        );

        if (!CryptoSystem::ChaCha20Poly1305::encrypt(
            key,
            nonce,
            inner,
            static_cast<unsigned long>(innerLength),
            header,
            5,
            ciphertext,
            tag
        ))
            return false;

        unsigned char record[16385 + 16 + 5];
        memcpy(record, header, 5);
        memcpy(record + 5, ciphertext, innerLength);
        memcpy(record + 5 + innerLength, tag, 16);

        const std::size_t recordLength = 5 + innerLength + 16;

        if (!socket.send(
            reinterpret_cast<char *>(record),
            static_cast<unsigned long>(recordLength)
        ))
            return false;

        ++sequence;
        return true;
    }

    /* ================================================================
     * Read complete TLS record
     * ================================================================ */

    bool TlsClient::readRecord(
        unsigned char *buffer,
        std::size_t capacity,
        std::size_t &length
    ) {
        length = 0;

        while (length < 5) {
            const long n = socket.receive(
                reinterpret_cast<char *>(buffer + length),
                static_cast<unsigned long>(5 - length)
            );

            if (n <= 0)
                return false;

            length += static_cast<std::size_t>(n);
        }

        const std::uint16_t recordLength = readU16(buffer + 3);

        if (recordLength > capacity - 5)
            return false;

        while (
            length <
            static_cast<std::size_t>(5 + recordLength)
        ) {
            const long n = socket.receive(
                reinterpret_cast<char *>(buffer + length),
                static_cast<unsigned long>(
                    5 + recordLength - length
                )
            );

            if (n <= 0)
                return false;

            length += static_cast<std::size_t>(n);
        }

        return true;
    }

    /* ================================================================
     * connect()
     * ================================================================ */

    bool TlsClient::connect(
        const char *host,
        unsigned short port
    ) {
        close();

        if (!socket.open())
            return false;

        if (!socket.connect(host, port)) {
            socket.close();
            return false;
        }

        unsigned char clientHello[2048];
        std::size_t clientHelloLength = 0;

        if (!buildClientHello(
            host,
            clientHello,
            sizeof(clientHello),
            clientHelloLength
        )) {
            close();
            return false;
        }

        if (!appendTranscript(clientHello, clientHelloLength)) {
            close();
            return false;
        }

        if (!socket.send(
            reinterpret_cast<char *>(clientHello),
            static_cast<unsigned long>(clientHelloLength)
        )) {
            close();
            return false;
        }

        unsigned char record[MAX_RECORD_SIZE];
        std::size_t recordLength = 0;

        if (!readRecord(record, sizeof(record), recordLength)) {
            close();
            return false;
        }

        if (record[0] != TLS_HANDSHAKE || record[5] != HS_SERVER_HELLO) {
            close();
            return false;
        }

        if (!appendTranscript(record, recordLength)) {
            close();
            return false;
        }

        if (!parseServerHello(record, recordLength)) {
            close();
            return false;
        }

        if (!CryptoSystem::X25519::computeSharedSecret(
            clientPrivateKey,
            serverPublicKey,
            sharedSecret
        )) {
            close();
            return false;
        }

        if (!deriveHandshakeKeys()) {
            close();
            return false;
        }

        if (!processServerHandshake(nullptr, 0)) {
            close();
            return false;
        }

        return true;
    }

    bool TlsClient::processServerHandshake(const unsigned char *, std::size_t) {
        return false;
    }

    bool TlsClient::processHandshakeMessage(const unsigned char *, std::size_t) { return false; }
    bool TlsClient::processEncryptedExtensions(const unsigned char *, std::size_t) { return false; }
    bool TlsClient::processCertificate(const unsigned char *, std::size_t) { return false; }
    bool TlsClient::processCertificateVerify(const unsigned char *, std::size_t) { return false; }
    bool TlsClient::processServerFinished(const unsigned char *, std::size_t) { return false; }
    bool TlsClient::sendFinished() { return false; }
    bool TlsClient::calculateFinishedVerifyData(const unsigned char [32], unsigned char [32]) { return false; }
    bool TlsClient::deriveApplicationKeys() { return false; }

    bool TlsClient::sendHttpRequest(
        const char *requestData,
        unsigned long len
    ) {
        if (!handshakeCompleted || !applicationKeysReady)
            return false;

        return encryptRecord(
            TLS_APPLICATION_DATA,
            reinterpret_cast<const unsigned char *>(requestData),
            static_cast<std::size_t>(len),
            appKey,
            appIv,
            clientSequence
        );
    }

    long TlsClient::receiveResponse(
        char *buffer,
        unsigned long size
    ) {
        if (!handshakeCompleted || !applicationKeysReady)
            return -1;

        unsigned char record[MAX_RECORD_SIZE];
        std::size_t recordLength = 0;

        if (!readRecord(record, sizeof(record), recordLength))
            return -1;

        unsigned char plaintext[MAX_RECORD_SIZE];
        std::size_t plaintextLength = 0;
        unsigned char contentType = 0;

        if (!decryptRecord(
            record,
            recordLength,
            serverAppKey,
            serverAppIv,
            serverSequence,
            plaintext,
            sizeof(plaintext),
            plaintextLength,
            contentType
        ))
            return -1;

        ++serverSequence;

        if (contentType != TLS_APPLICATION_DATA)
            return -1;

        if (plaintextLength > size)
            plaintextLength = size;

        memcpy(buffer, plaintext, plaintextLength);

        return static_cast<long>(plaintextLength);
    }

    void TlsClient::close() {
        socket.close();

        handshakeCompleted = false;
        handshakeKeysReady = false;
        applicationKeysReady = false;

        transcriptLength = 0;
        clientSequence = 0;
        serverSequence = 0;
    }
} // namespace NetworkSystem
