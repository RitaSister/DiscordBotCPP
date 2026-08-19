#include "console/Console.h"
#include "network/Network.h"
#include "config/Config.h"
#include "scheduler/Scheduler.h"
#include "logging/debug/Debug.h"

extern "C" void mainCRTStartup() {
    ConsoleSystem::ConsoleContext console = ConsoleSystem::init();

    DebugSystem::log(console.hOut, DebugSystem::LogLevel::INFO, "Starting bot initialization...");

    // 1. Инициализация сети
    if (NetworkSystem::initNetwork()) {
        DebugSystem::log(console.hOut, DebugSystem::LogLevel::INFO, "Network initialized successfully!");
    } else {
        DebugSystem::log(console.hOut, DebugSystem::LogLevel::ERROR_LVL, "Failed to initialize network.");
    }

    // 2. Чтение конфига
    ConfigSystem::BotConfig cfg = ConfigSystem::loadConfig("config.yml");
    if (cfg.token[0] != '\0') {
        DebugSystem::log(console.hOut, DebugSystem::LogLevel::INFO, "Config loaded! Token found.");
    } else {
        DebugSystem::log(console.hOut, DebugSystem::LogLevel::WARNING, "config.yml not found or token is empty. Default template created.");
    }

    DebugSystem::log(console.hOut, DebugSystem::LogLevel::DEBUG, "Entering main event loop with scheduler.");

    // 3. Шедулер
    SchedulerSystem::Task fastTask(3000);

    while (true) {
        unsigned long long currentTime = SchedulerSystem::getSystemTimeMs();

        if (fastTask.check(currentTime)) {
            DebugSystem::log(console.hOut, DebugSystem::LogLevel::DEBUG, "Scheduler tick: background check...");
        }

        SchedulerSystem::systemSleep(10);
    }

    NetworkSystem::cleanupNetwork();
    ConsoleSystem::shutdown(0);
}
