#include "Debug.h"
#include "../../console/Console.h"
#include "../../util/TimeUtils.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall CreateDirectoryA(const char*, void*);
    __declspec(dllimport) unsigned long __stdcall SetFilePointer(void*, long, long*, unsigned long);
    __declspec(dllimport) int __stdcall CloseHandle(void*);
}
#elif __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace DebugSystem {

    void Logger::writeToFile(const char* text, unsigned long size) {
        const char* dirName = "logs";
        const char* filename = "logs/latest.log";

#ifdef _WIN32
        CreateDirectoryA(dirName, nullptr);

        void* hFile = CreateFileA(filename, 0x40000000, 1, nullptr, 4, 0x00000080, nullptr);
        if (hFile != reinterpret_cast<void*>(-1)) {
            unsigned long written = 0;
            SetFilePointer(hFile, 0, nullptr, 2); // 2 = FILE_END
            WriteFile(hFile, text, size, &written, nullptr);
            CloseHandle(hFile);
        }
#elif __linux__
        mkdir(dirName, 0755);
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

        idx += Utils::TimeUtils::getFormattedTime(logBuffer + idx, sizeof(logBuffer) - idx);

        const char* lvlStr = "[INFO] ";
        switch (level) {
            case LogLevel::INFO:      lvlStr = "[INFO] "; break;
            case LogLevel::WARNING:   lvlStr = "[WARNING] "; break;
            case LogLevel::ERROR: lvlStr = "[ERROR] "; break;
            case LogLevel::DEBUG:     lvlStr = "[DEBUG] "; break;
        }

        int l = 0;
        while (lvlStr[l] != '\0' && idx < sizeof(logBuffer) - 2) {
            logBuffer[idx++] = lvlStr[l++];
        }

        int m = 0;
        while (message[m] != '\0' && idx < sizeof(logBuffer) - 2) {
            logBuffer[idx++] = message[m++];
        }

        logBuffer[idx++] = '\n';
        logBuffer[idx] = '\0';

        if (hOut) {
            ConsoleSystem::write(hOut, logBuffer, idx);
        }
        writeToFile(logBuffer, idx);
    }

} // namespace DebugSystem
