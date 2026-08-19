#include "Debug.h"
#include "../../console/Console.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
    // Структура для системного времени Windows
    struct SYSTEMTIME {
        unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
    };
    __declspec(dllimport) void __stdcall GetLocalTime(SYSTEMTIME* lpSystemTime);
}
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#endif

namespace DebugSystem {

    void writeToFile(const char* text, unsigned long size) {
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

    // Вспомогательная функция для добавления ведущего нуля (например, "05" вместо "5")
    void appendTwoDigits(char* buf, int& index, int value) {
        buf[index++] = '0' + (value / 10);
        buf[index++] = '0' + (value % 10);
    }

    void log(void* hOut, LogLevel level, const char* message) {
        if (!message) return;

        char logBuffer[512];
        int idx = 0;

        // Получаем реальное системное время
        int hour = 0, minute = 0, second = 0;

#ifdef _WIN32
        SYSTEMTIME st;
        GetLocalTime(&st);
        hour = st.wHour;
        minute = st.wMinute;
        second = st.wSecond;
#elif __linux__
        time_t rawtime;
        time(&rawtime);
        struct tm* timeinfo = localtime(&rawtime);
        if (timeinfo) {
            hour = timeinfo->tm_hour;
            minute = timeinfo->tm_min;
            second = timeinfo->tm_sec;
        }
#endif

        // 1. Форматируем таймстамп в виде [HH:MM:SS]
        logBuffer[idx++] = '[';
        appendTwoDigits(logBuffer, idx, hour);
        logBuffer[idx++] = ':';
        appendTwoDigits(logBuffer, idx, minute);
        logBuffer[idx++] = ':';
        appendTwoDigits(logBuffer, idx, second);
        logBuffer[idx++] = ']';
        logBuffer[idx++] = ' ';

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
