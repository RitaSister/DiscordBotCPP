#include "Network.h"

// ОПРЕДЕЛЕНИЕ ДЛЯ WINDOWS
#ifdef _WIN32
extern "C" {
__declspec(dllimport) int __stdcall WSAStartup(unsigned short wVersionRequested, void* lpWSAData);
__declspec(dllimport) int __stdcall WSACleanup();
}
#endif

namespace NetworkSystem {

    bool initNetwork() {
#ifdef _WIN32
        // В Windows обязательно нужно запустить подсистему Winsock перед работой с сокетами
        char wsaData[512];
        const int result = WSAStartup(0x0202, &wsaData); // Версия 2.2
        return (result == 0);
#elif __linux__
        // В Linux сетевые функции встроены в ядро, инициализация не требуется
        return true;
#else
        return false;
#endif
    }

    void cleanupNetwork() {
#ifdef _WIN32
        // Выключаем Winsock при завершении программы в Windows
        WSACleanup();
#elif __linux__
        // В Linux ничего чистить не нужно
#endif
    }

} // namespace NetworkSystem
