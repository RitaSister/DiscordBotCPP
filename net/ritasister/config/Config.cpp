#include "Config.h"

// Кросс-платформенные системные функции для работы с файлами
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall ReadFile(void*, void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
}
#elif __linux__
extern "C" {
    int open(const char *pathname, int flags, int mode);
    long read(int fd, void *buf, unsigned long count);
    long write(int fd, const void *buf, unsigned long count);
    int close(int fd);
}
#endif

namespace ConfigSystem {

    bool createDefaultConfig(const char* filename) {
        const char* templateContent = "token: \"YOUR_BOT_TOKEN_HERE\"\n";
        unsigned long contentLength = 0;
        while (templateContent[contentLength] != '\0') {
            contentLength++;
        }

        bool success = false;

#ifdef _WIN32
        // CREATE_ALWAYS (значение 2) — создает новый файл или перезаписывает существующий
        void* hFile = CreateFileA(filename, 0x40000000 /* GENERIC_WRITE */, 0, nullptr, 2 /* CREATE_ALWAYS */, 0x00000080 /* FILE_ATTRIBUTE_NORMAL */, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long written = 0;
            WriteFile(hFile, templateContent, contentLength, &written, nullptr);
            success = (written == contentLength);
            // Закрываем дескриптор через CloseHandle (можно импортировать или оставить утечку на момент выхода, но лучше закрывать)
        }
#elif __linux__
        // O_WRONLY | O_CREAT | O_TRUNC, права 0644
        int fd = open(filename, 01 | 0100 | 01000, 0644);
        if (fd >= 0) {
            long written = write(fd, templateContent, contentLength);
            close(fd);
            success = (written == static_cast<long>(contentLength));
        }
#endif

        return success;
    }

    BotConfig loadConfig(const char* filename) {
        BotConfig config = {};
        char fileBuffer[512] = {0};
        long bytesRead = 0;

#ifdef _WIN32
        void* hFile = CreateFileA(filename, 0x80000000 /* GENERIC_READ */, 1 /* FILE_SHARE_READ */, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long readTotal = 0;
            ReadFile(hFile, fileBuffer, sizeof(fileBuffer) - 1, &readTotal, nullptr);
            bytesRead = static_cast<long>(readTotal);
        }
#elif __linux__
        int fd = open(filename, 0, 0); // O_RDONLY
        if (fd >= 0) {
            bytesRead = read(fd, fileBuffer, sizeof(fileBuffer) - 1);
            close(fd);
        }
#endif

        // Если файла нет, автоматически создаем шаблон
        if (bytesRead <= 0) {
            createDefaultConfig(filename);
            return config;
        }

        // Простейший парсер token:
        for (long i = 0; i < bytesRead - 6; ++i) {
            if (fileBuffer[i] == 't' && fileBuffer[i+1] == 'o' && fileBuffer[i+2] == 'k' &&
                fileBuffer[i+3] == 'e' && fileBuffer[i+4] == 'n' && fileBuffer[i+5] == ':') {

                long j = i + 6;
                while (j < bytesRead && (fileBuffer[j] == ' ' || fileBuffer[j] == '"' || fileBuffer[j] == '\'')) {
                    j++;
                }

                int tokenIdx = 0;
                while (j < bytesRead && fileBuffer[j] != '"' && fileBuffer[j] != '\'' && fileBuffer[j] != '\r' && fileBuffer[j] != '\n' && tokenIdx < sizeof(config.token) - 1) {
                    config.token[tokenIdx++] = fileBuffer[j++];
                }
                config.token[tokenIdx] = '\0';
                break;
            }
        }

        return config;
    }

} // namespace ConfigSystem
