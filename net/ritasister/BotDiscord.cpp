#include "BotDiscord.h"
#include "logging/debug/Debug.h"
#include "config/Config.h"
#include "logging/LogArchiver.h"
#include "network/Network.h"
#include "network/http/server/HttpServer.h"
#include "scheduler/Scheduler.h"
#include "discord/DiscordClient.h"

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
        hOut = CreateFileA("CONOUT$", 0x40000000 /* GENERIC_WRITE */, 2, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
        hIn  = CreateFileA("CONIN$",  0x80000000 /* GENERIC_READ */,  1, nullptr, 3 /* OPEN_EXISTING */, 0, nullptr);
        if (hOut == reinterpret_cast<void*>(-1)) {
            ExitProcess(1);
        }
#elif __linux__
        hOut = reinterpret_cast<void*>(1L);
        hIn  = reinterpret_cast<void*>(0L);
#endif
    }

    void BotDiscord::start() {
        LogSystem::LogArchiver::archiveOldLog();
        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Starting bare-metal bot initialization...");

        // Загрузка конфигурации
        if (const ConfigSystem::BotConfig cfg = ConfigSystem::loadConfig("config.yml"); cfg.token[0] != '\0' && !ConfigSystem::isDefaultToken(cfg)) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Config loaded! Token found.");
        } else {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::WARNING, "config.yml has default placeholder. Please set your real token.");
        }

        // 1. Инициализируем сетевую подсистему один раз на старте
        if (!NetworkSystem::initNetwork()) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "Failed to initialize network subsystem.");
            return;
        }

        // 2. Запускаем локальный сервер
        NetworkSystem::HttpServer localServer(8080);
        if (localServer.start()) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Local HttpServer started successfully on port 8080.");
        } else {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "Failed to start local HttpServer.");
        }

        // Даем серверу 50 мс на инициализацию слушающего сокета
        SchedulerSystem::systemSleep(50);

        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::DEBUG, "Entering main event loop with scheduler.");
        isRunning = true;

        // Главный цикл событий (Event Loop) на шедулере
        SchedulerSystem::Task fastTask(3000);
        SchedulerSystem::Task serverPollTask(50);

        bool requestSent = false; // Флаг для отправки запроса ровно один раз

        while (isRunning) {
            unsigned long long currentTime = SchedulerSystem::getSystemTimeMs();

            if (fastTask.check(currentTime)) {
                DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::DEBUG, "Scheduler tick: background check...");
            }

            // Опрашиваем наш HTTP-сервер
            if (serverPollTask.check(currentTime)) {
                localServer.pollAndHandle(hOut);

                // Отправляем запрос сразу, как только шедулер начал опрос сервера
                if (!requestSent) {
                    Core::sendDiscordApiRequest(hOut);
                    requestSent = true;
                }
            }

            SchedulerSystem::systemSleep(10);
        }

        // Корректная очистка ресурсов происходит ТОЛЬКО при завершении работы бота
        localServer.stop();
        NetworkSystem::cleanupNetwork();
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
