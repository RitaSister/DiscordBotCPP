#pragma once

namespace NetworkSystem {

    class HttpServer {
    private:
        void* serverSocketHandle;
        unsigned short port;
        bool running;

    public:
        explicit HttpServer(unsigned short serverPort);
        ~HttpServer();

        bool start();
        void pollAndHandle(void* hOut); // Опрос входящих подключений (non-blocking)
        void stop();
    };

} // namespace NetworkSystem
