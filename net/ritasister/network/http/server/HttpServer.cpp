#include "HttpServer.h"
#include "../../../console/Console.h"
#include "../../../logging/debug/Debug.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) unsigned long long __stdcall socket(int af, int type, int protocol);
    __declspec(dllimport) int __stdcall bind(unsigned long long s, const void* name, int namelen);
    __declspec(dllimport) int __stdcall listen(unsigned long long s, int backlog);
    __declspec(dllimport) unsigned long long __stdcall accept(unsigned long long s, void* addr, int* addrlen);
    __declspec(dllimport) int __stdcall recv(unsigned long long s, char* buf, int len, int flags);
    __declspec(dllimport) int __stdcall send(unsigned long long s, const char* buf, int len, int flags);
    __declspec(dllimport) int __stdcall closesocket(unsigned long long s);
    __declspec(dllimport) unsigned short __stdcall htons(unsigned short hostshort);
    __declspec(dllimport) unsigned long __stdcall inet_addr(const char* cp);
    // Для неблокирующего режима (ioctlsocket)
    __declspec(dllimport) int __stdcall ioctlsocket(unsigned long long s, long cmd, unsigned long* argp);
}
#endif

namespace NetworkSystem {

    HttpServer::HttpServer(unsigned short serverPort) 
        : serverSocketHandle(nullptr), port(serverPort), running(false) {}

    HttpServer::~HttpServer() {
        stop();
    }

    bool HttpServer::start() {
#ifdef _WIN32
        unsigned long long s = socket(2, 1, 6); // AF_INET, SOCK_STREAM, IPPROTO_TCP
        if (s == ~0ULL) return false;

        char addr[16] = {0};
        *reinterpret_cast<unsigned short*>(addr) = 2; // AF_INET
        *reinterpret_cast<unsigned short*>(addr + 2) = htons(port);
        *reinterpret_cast<unsigned long*>(addr + 4) = inet_addr("127.0.0.1");

        // Сначала выполняем bind в обычном блокирующем режиме
        if (bind(s, addr, 16) == -1) {
            closesocket(s);
            return false;
        }

        // Затем ставим сокет в режим прослушивания
        if (listen(s, 5) == -1) {
            closesocket(s);
            return false;
        }

        // И только ПОСЛЕ успешного listen включаем неблокирующий режим для accept,
        // чтобы сервер не вешал главный поток при опросе
        unsigned long mode = 1; // 1 для неблокирующего
        ioctlsocket(s, 0x8066637e /* FIONBIO */, &mode);

        serverSocketHandle = reinterpret_cast<void*>(s);
        running = true;
        return true;
#else
        return false;
#endif
    }

    void HttpServer::pollAndHandle(void* hOut) {
        if (!running || !serverSocketHandle) return;

#ifdef _WIN32
        unsigned long long serverS = reinterpret_cast<unsigned long long>(serverSocketHandle);

        int addrLen = 16;
        char clientAddr[16] = {0};

        // Пытаемся принять входящее подключение (неблокирующий вызов)
        unsigned long long clientS = accept(serverS, clientAddr, &addrLen);
        if (clientS != ~0ULL /* INVALID_SOCKET */) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "HttpServer: Client connected! Reading payload...");

            char requestBuffer[1024] = {0};
            int bytesReceived = recv(clientS, requestBuffer, sizeof(requestBuffer) - 1, 0);

            if (bytesReceived > 0) {
                DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "HttpServer received request:");
                ConsoleSystem::write(hOut, requestBuffer);

                // Отправляем фейковый JSON-ответ Discord API (например, данные о боте)
                const char* httpResponse =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n\r\n"
                    "{\"id\":\"123456789\",\"username\":\"BareMetalBot\",\"bot\":true}";

                send(clientS, httpResponse, 114, 0);
            }

            closesocket(clientS);
        }
#endif
    }

    void HttpServer::stop() {
        if (serverSocketHandle) {
#ifdef _WIN32
            unsigned long long s = reinterpret_cast<unsigned long long>(serverSocketHandle);
            closesocket(s);
#endif
            serverSocketHandle = nullptr;
            running = false;
        }
    }

} // namespace NetworkSystem
