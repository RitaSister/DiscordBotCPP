#include "LogArchiver.h"
#include "../util/TimeUtils.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall ReadFile(void*, void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall CloseHandle(void*);
    __declspec(dllimport) int __stdcall CreateDirectoryA(const char*, void*);
    __declspec(dllimport) int __stdcall DeleteFileA(const char*);
}
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#endif

namespace LogSystem {

    // Табличный расчет CRC32 по стандарту IEEE 802.3
    static unsigned int calculateCRC32(const unsigned char* buffer, unsigned long size) {
        unsigned int crc = 0xFFFFFFFF;
        for (unsigned long i = 0; i < size; ++i) {
            crc = crc ^ buffer[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                } else {
                    crc = crc >> 1;
                }
            }
        }
        return ~crc;
    }

    void LogArchiver::archiveOldLog() {
        const char* currentLog = "logs/latest.log";

#ifdef _WIN32
        void* hSource = CreateFileA(currentLog, 0x80000000, 1, nullptr, 3, 0, nullptr);
        if (hSource == reinterpret_cast<void*>(-1)) {
            return;
        }

        static char timeStr[64];
        Utils::TimeUtils::getFormattedTime(timeStr, sizeof(timeStr));
        for (int i = 0; timeStr[i] != '\0'; ++i) {
            if (timeStr[i] == ':' || timeStr[i] == ' ' || timeStr[i] == '[' || timeStr[i] == ']') {
                timeStr[i] = '_';
            }
        }

        CreateDirectoryA("logs/archive", nullptr);

        static char gzPath[256];
        const char* prefix = "logs/archive/log_";
        int idx = 0;
        while (prefix[idx] != '\0') {
            gzPath[idx] = prefix[idx];
            idx++;
        }

        for (int i = 0; timeStr[i] != '\0' && idx < sizeof(gzPath) - 7; ++i) {
            if (timeStr[i] != '\r' && timeStr[i] != '\n') {
                gzPath[idx++] = timeStr[i];
            }
        }
        const char* ext = ".log.gz";
        for (int i = 0; ext[i] != '\0' && idx < sizeof(gzPath) - 1; ++i) {
            gzPath[idx++] = ext[i];
        }
        gzPath[idx] = '\0';

        void* hGz = CreateFileA(gzPath, 0x40000000, 0, nullptr, 2, 0x00000080, nullptr);
        if (hGz != reinterpret_cast<void*>(-1)) {
            unsigned long written = 0;

            // 1. Заголовок GZIP (RFC 1952)
            unsigned char gzHeader[10] = { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
            WriteFile(hGz, gzHeader, 10, &written, nullptr);

            static char smallBuffer[32768]; // Буфер по 32 КБ для блоков Stored DEFLATE
            unsigned long totalBytesRead = 0;
            unsigned long bytesRead = 0;
            unsigned int crc = 0xFFFFFFFF;

            while (ReadFile(hSource, smallBuffer, sizeof(smallBuffer), &bytesRead, nullptr) && bytesRead > 0) {
                // Считаем CRC32
                for (unsigned long k = 0; k < bytesRead; ++k) {
                    crc = crc ^ static_cast<unsigned char>(smallBuffer[k]);
                    for (int j = 0; j < 8; ++j) {
                        if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                        else crc = crc >> 1;
                    }
                }

                // Записываем DEFLATE Stored-блок заголовок для каждого куска (до 64К)
                // BFINAL = 1 (последний блок), BTYPE = 00 (Stored без сжатия) -> байт 0x01
                unsigned char blockHeader[5];
                blockHeader[0] = 0x01; // Финальный блок, тип Stored
                unsigned short len = static_cast<unsigned short>(bytesRead);
                unsigned short nlen = ~len;

                // Записываем LEN (2 байта) и NLEN (2 байта) в формате Little-Endian
                blockHeader[1] = len & 0xFF;
                blockHeader[2] = (len >> 8) & 0xFF;
                blockHeader[3] = nlen & 0xFF;
                blockHeader[4] = (nlen >> 8) & 0xFF;

                WriteFile(hGz, blockHeader, 5, &written, nullptr);
                WriteFile(hGz, smallBuffer, bytesRead, &written, nullptr);

                totalBytesRead += bytesRead;
            }

            unsigned int finalCrc = ~crc;

            // Хвост GZIP: CRC32 и размер
            WriteFile(hGz, &finalCrc, 4, &written, nullptr);
            WriteFile(hGz, &totalBytesRead, 4, &written, nullptr);

            CloseHandle(hGz);
        }

        CloseHandle(hSource);
        DeleteFileA(currentLog);

#elif __linux__
        struct stat buffer;
        if (stat(currentLog, &buffer) != 0) {
            return;
        }

        FILE* fSource = fopen(currentLog, "rb");
        if (!fSource) return;

        mkdir("logs/archive", 0755);

        static char timeStr[64];
        Utils::TimeUtils::getFormattedTime(timeStr, sizeof(timeStr));
        for (int i = 0; timeStr[i] != '\0'; ++i) {
            if (timeStr[i] == ':' || timeStr[i] == ' ' || timeStr[i] == '[' || timeStr[i] == ']') {
                timeStr[i] = '_';
            }
        }

        static char gzPath[256] = "logs/archive/log_";
        int idx = 17;
        for (int i = 0; timeStr[i] != '\0' && idx < sizeof(gzPath) - 7; ++i) {
            if (timeStr[i] != '\r' && timeStr[i] != '\n') {
                gzPath[idx++] = timeStr[i];
            }
        }
        const char* ext = ".log.gz";
        for (int i = 0; ext[i] != '\0' && idx < sizeof(gzPath) - 1; ++i) {
            gzPath[idx++] = ext[i];
        }
        gzPath[idx] = '\0';

        FILE* fGz = fopen(gzPath, "wb");
        if (fGz) {
            unsigned char gzHeader[10] = { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
            fwrite(gzHeader, 1, 10, fGz);

            static char smallBuffer[32768];
            size_t totalBytesRead = 0;
            size_t bytesRead = 0;
            unsigned int crc = 0xFFFFFFFF;

            while ((bytesRead = fread(smallBuffer, 1, sizeof(smallBuffer), fSource)) > 0) {
                for (size_t k = 0; k < bytesRead; ++k) {
                    crc = crc ^ static_cast<unsigned char>(smallBuffer[k]);
                    for (int j = 0; j < 8; ++j) {
                        if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                        else crc = crc >> 1;
                    }
                }

                unsigned char blockHeader[5];
                blockHeader[0] = 0x01;
                unsigned short len = static_cast<unsigned short>(bytesRead);
                unsigned short nlen = ~len;
                blockHeader[1] = len & 0xFF;
                blockHeader[2] = (len >> 8) & 0xFF;
                blockHeader[3] = nlen & 0xFF;
                blockHeader[4] = (nlen >> 8) & 0xFF;

                fwrite(blockHeader, 1, 5, fGz);
                fwrite(smallBuffer, 1, bytesRead, fGz);

                totalBytesRead += bytesRead;
            }

            unsigned int finalCrc = ~crc;
            unsigned int uncompressedSize = static_cast<unsigned int>(totalBytesRead);

            fwrite(&finalCrc, 1, 4, fGz);
            fwrite(&uncompressedSize, 1, 4, fGz);
            fclose(fGz);
        }

        fclose(fSource);
        unlink(currentLog);
#endif
    }

} // namespace LogSystem
