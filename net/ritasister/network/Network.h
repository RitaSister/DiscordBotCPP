#pragma once

namespace NetworkSystem {
    // Инициализация сети (включает Winsock на Windows, на Linux возвращает true)
    bool initNetwork();

    // Очистка сетевых ресурсов при выходе из программы
    void cleanupNetwork();

    // Создание сырого TCP-сокета и подключение к IP/порту
    void* createTcpSocket();
    bool connectSocket(void* socketPtr, const char* ip, unsigned short port);
    void closeSocket(void* socketPtr);
}
