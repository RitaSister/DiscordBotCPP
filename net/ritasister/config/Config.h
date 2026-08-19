#pragma once

namespace ConfigSystem {
    struct BotConfig {
        char token[128];
        unsigned int flags;
        unsigned short port;
    };

    BotConfig loadConfig(const char* filename);
    bool isDefaultToken(const BotConfig& config); // Проверка на заглушку
}
