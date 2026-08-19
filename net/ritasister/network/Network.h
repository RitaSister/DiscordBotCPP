#pragma once

namespace NetworkSystem {
    // Инициализация сети (включает Winsock на Windows, на Linux возвращает true)
    bool initNetwork();

    // Очистка сетевых ресурсов при выходе из программы
    void cleanupNetwork();
}
