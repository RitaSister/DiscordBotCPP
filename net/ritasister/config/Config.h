#pragma once

namespace ConfigSystem {
    struct BotConfig {
        char token[128];
    };

    // Чтение конфигурационного файла
    BotConfig loadConfig(const char* filename);

    // Создание файла конфигурации по умолчанию с шаблоном
    bool createDefaultConfig(const char* filename);
}
