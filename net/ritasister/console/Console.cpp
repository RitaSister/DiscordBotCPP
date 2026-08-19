#include "Console.h"

// ОПРЕДЕЛЕНИЕ ДЛЯ WINDOWS
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) int __stdcall ReadFile(void*, void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) int __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
    __declspec(dllimport) void __stdcall ExitProcess(unsigned int);
}
#endif

// ОПРЕДЕЛЕНИЕ ДЛЯ LINUX (Debian)
#ifdef __linux__
extern "C" {
    long write(int fd, const void *buf, unsigned long count);
    long read(int fd, void *buf, unsigned long count);
    void _exit(int status);
}
#endif

namespace ConsoleSystem {

    ConsoleContext init() {
#ifdef _WIN32
        void* hOut = CreateFileA("CONOUT$", 0x40000000, 2, nullptr, 3, 0, nullptr);
        void* hIn  = CreateFileA("CONIN$",  0x80000000, 1, nullptr, 3, 0, nullptr);
        if (hOut == reinterpret_cast<void*>(-1) || hIn == reinterpret_cast<void*>(-1)) {
            ExitProcess(1);
        }
        return { hOut, hIn };
#elif __linux__
        return {
            reinterpret_cast<void*>(1), reinterpret_cast<void*>(0)
        };
#endif
    }

    void write(void* hOut, const char* text, unsigned long size) {
#ifdef _WIN32
        unsigned long written = 0;
        WriteFile(hOut, text, size, &written, nullptr);
#elif __linux__
        int fd = static_cast<int>(reinterpret_cast<long>(hOut));
        ::write(fd, text, size);
#endif
    }

    // Улучшение: удобная перегрузка для вывода обычной строки без ручного указания размера
    void write(void* hOut, const char* text) {
        if (!text) return;
        unsigned long size = 0;
        while (text[size] != '\0') {
            size++;
        }
        write(hOut, text, size);
    }

    long read(void* hIn, char* buffer, unsigned long size) {
#ifdef _WIN32
        unsigned long bytesRead = 0;
        if (ReadFile(hIn, buffer, size, &bytesRead, nullptr)) {
            return static_cast<long>(bytesRead);
        }
        return -1;
#elif __linux__
        int fd = static_cast<int>(reinterpret_cast<long>(hIn));
        long bytes = ::read(fd, buffer, size);
        return bytes;
#endif
    }

    void shutdown(int exitCode) {
#ifdef _WIN32
        ExitProcess(exitCode);
#elif __linux__
        _exit(exitCode);
#endif
    }

} // namespace ConsoleSystem
