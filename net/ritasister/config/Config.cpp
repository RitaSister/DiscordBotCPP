#include "Config.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall ReadFile(void*, void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
}
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#endif

namespace ConfigSystem {
    static bool createDefaultConfig(const char* filename) {
        // Человекочитаемый YAML-шаблон
        const char* templateContent = "token: \"YOUR_BOT_TOKEN_HERE\"\n";
        unsigned long contentLength = 0;
        while (templateContent[contentLength] != '\0') {
            contentLength++;
        }

        bool success = false;
#ifdef _WIN32
        void* hFile = CreateFileA(filename, 0x40000000, 0, nullptr, 2, 0x00000080, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long written = 0;
            WriteFile(hFile, templateContent, contentLength, &written, nullptr);
            success = (written == contentLength);
        }
#elif __linux__
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
        // Заполняем дефолтными значениями структуру
        const char* defaultPlaceholder = "YOUR_BOT_TOKEN_HERE";
        for (int i = 0; defaultPlaceholder[i] != '\0' && i < sizeof(config.token) - 1; ++i) {
            config.token[i] = defaultPlaceholder[i];
        }
        config.flags = 1;
        config.port = 443;

        char fileBuffer[512] = {0};
        long bytesRead = 0;

#ifdef _WIN32
        void* hFile = CreateFileA(filename, 0x80000000, 1, nullptr, 3, 0, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long readTotal = 0;
            ReadFile(hFile, fileBuffer, sizeof(fileBuffer) - 1, &readTotal, nullptr);
            bytesRead = static_cast<long>(readTotal);
        }
#elif __linux__
        int fd = open(filename, 0, 0);
        if (fd >= 0) {
            bytesRead = read(fd, fileBuffer, sizeof(fileBuffer) - 1);
            close(fd);
        }
#endif

        // Если файла нет или он пустой — создаем шаблон
        if (bytesRead <= 0) {
            createDefaultConfig(filename);
            return config; // Возвращаем дефолтную структуру с заглушкой
        }

        // Простейший низкоуровневый парсер строки token: в YAML
        for (long i = 0; i < bytesRead - 6; ++i) {
            if (fileBuffer[i] == 't' && fileBuffer[i+1] == 'o' && fileBuffer[i+2] == 'k' &&
                fileBuffer[i+3] == 'e' && fileBuffer[i+4] == 'n' && fileBuffer[i+5] == ':') {

                long j = i + 6;
                while (j < bytesRead && (fileBuffer[j] == ' ' || fileBuffer[j] == '"' || fileBuffer[j] == '\'')) {
                    j++;
                }

                int tokenIdx = 0;
                while (j < bytesRead && fileBuffer[j] != '"' && fileBuffer[j] != '\'' &&
                       fileBuffer[j] != '\r' && fileBuffer[j] != '\n' && tokenIdx < sizeof(config.token) - 1) {
                    config.token[tokenIdx++] = fileBuffer[j++];
                }
                config.token[tokenIdx] = '\0';
                break;
            }
        }

        return config;
    }

    bool isDefaultToken(const BotConfig& config) {
        const char* defaultToken = "YOUR_BOT_TOKEN_HERE";
        int i = 0;
        while (i < sizeof(config.token)) {
            if (config.token[i] != defaultToken[i]) {
                return false;
            }
            if (defaultToken[i] == '\0') {
                return true;
            }
            i++;
        }
        return true;
    }

} // namespace ConfigSystem
