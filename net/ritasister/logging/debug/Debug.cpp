#include "Debug.h"
#include "../../console/Console.h"
#include "../../util/TimeUtils.h" // Подключаем наш новый класс времени

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
}
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#endif

namespace DebugSystem {

    void Logger::writeToFile(const char* text, unsigned long size) {
        const char* filename = "bot.log";
#ifdef _WIN32
        void* hFile = CreateFileA(filename, 0x00000004, 1, nullptr, 3, 0x00000080, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long written = 0;
            WriteFile(hFile, text, size, &written, nullptr);
        }
#elif __linux__
        int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd >= 0) {
            write(fd, text, size);
            close(fd);
        }
#endif
    }

    void Logger::log(void* hOut, LogLevel level, const char* message) {
        if (!message) return;

        char logBuffer[512];
        int idx = 0;

        // 1. Получаем готовый таймстамп через утилитный класс
        idx += Utils::TimeUtils::getFormattedTime(logBuffer + idx, sizeof(logBuffer) - idx);

        // 2. Уровень логирования
        const char* lvlStr = "[INFO] ";
        switch (level) {
            case LogLevel::INFO:      lvlStr = "[INFO] "; break;
            case LogLevel::WARNING:   lvlStr = "[WARNING] "; break;
            case LogLevel::ERROR_LVL: lvlStr = "[ERROR] "; break;
            case LogLevel::DEBUG:     lvlStr = "[DEBUG] "; break;
        }

        int l = 0;
        while (lvlStr[l] != '\0' && idx < sizeof(logBuffer) - 2) {
            logBuffer[idx++] = lvlStr[l++];
        }

        // 3. Сообщение
        int m = 0;
        while (message[m] != '\0' && idx < sizeof(logBuffer) - 2) {
            logBuffer[idx++] = message[m++];
        }

        logBuffer[idx++] = '\n';
        logBuffer[idx] = '\0';

        // Вывод в консоль и файл
        if (hOut) {
            ConsoleSystem::write(hOut, logBuffer, idx);
        }
        writeToFile(logBuffer, idx);
    }

} // namespace DebugSystem
