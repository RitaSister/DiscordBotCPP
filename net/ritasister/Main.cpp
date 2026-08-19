#include "BotDiscord.h"

// «Голая» точка входа в процессор (минуя CRT)
extern "C" void mainCRTStartup() {
    Core::BotDiscord bot;
    bot.start();
    bot.stop(0);
}
