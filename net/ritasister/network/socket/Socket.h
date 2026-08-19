#pragma once

namespace NetworkSystem {

    class Socket {
    private:
        void* socketHandle; // Сырой дескриптор сокета
        bool connected;

    public:
        Socket();
        ~Socket();

        // Запрещаем копирование, чтобы случайно не закрыть один сокет дважды
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        // Основные методы работы
        bool open();
        bool connect(const char* ip, unsigned short port);
        bool send(const char* data, unsigned long size);
        long receive(char* buffer, unsigned long size);
        void close();

        bool isConnected() const;
    };

} // namespace NetworkSystem
