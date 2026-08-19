#include "BotDiscord.h"

inline void* operator new(unsigned long long, void* ptr) noexcept {
    return ptr;
}

alignas(Core::BotDiscord) static char botMemory[sizeof(Core::BotDiscord)];

// Кастомное имя точки входа вместо стандартного mainCRTStartup
extern "C" void _discord_bot_entry() {
    Core::BotDiscord* bot = new (botMemory) Core::BotDiscord();
    bot->start();
    bot->stop(0);
}
