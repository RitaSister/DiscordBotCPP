#include "BotDiscord.h"
#include "logging/debug/Debug.h"
#include "config/Config.h"
#include "logging/LogArchiver.h"
#include "scheduler/Scheduler.h"

// Прямые системные объявления функций ОС
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void*);
    __declspec(dllimport) void __stdcall ExitProcess(unsigned int);
}
#elif __linux__
extern "C" {
    void _exit(int status);
}
#endif

namespace Core {

    BotDiscord::BotDiscord() : hOut(nullptr), hIn(nullptr), isRunning(false) {
#ifdef _WIN32
        // Прямое получение дескриптора консоли на уровне ядра ОС
        hOut = CreateFileA("CONOUT$", 0x40000000 /* GENERIC_WRITE */, 2, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
        hIn  = CreateFileA("CONIN$",  0x80000000 /* GENERIC_READ */,  1, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
        if (hOut == reinterpret_cast<void*>(-1)) {
            ExitProcess(1);
        }
#elif __linux__
        // В Linux стандартные дескрипторы: stdout = 1, stdin = 0
        hOut = reinterpret_cast<void*>(1L);
        hIn  = reinterpret_cast<void*>(0L);
#endif
    }

    void BotDiscord::start() {
        LogSystem::LogArchiver::archiveOldLog();
        // 1. Старт логирования через наш полноценный класс Logger
        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Starting bare-metal bot initialization...");

        // 2. Загрузка конфигурации
        if (const ConfigSystem::BotConfig cfg = ConfigSystem::loadConfig("config.yml"); cfg.token[0] != '\0' && !ConfigSystem::isDefaultToken(cfg)) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Config loaded! Token found.");
        } else {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::WARNING, "config.yml has default placeholder. Please set your real token.");
        }

        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::DEBUG, "Entering main event loop with scheduler.");
        isRunning = true;

        // 3. Главный цикл событий (Event Loop) на шедулере
        SchedulerSystem::Task fastTask(3000);

        while (isRunning) {
            unsigned long long currentTime = SchedulerSystem::getSystemTimeMs();

            if (fastTask.check(currentTime)) {
                DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::DEBUG, "Scheduler tick: background check...");
            }

            SchedulerSystem::systemSleep(10);
        }
    }

    void BotDiscord::stop(int exitCode) {
        isRunning = false;
#ifdef _WIN32
        ExitProcess(exitCode);
#elif __linux__
        _exit(exitCode);
#endif
    }

} // namespace Core
