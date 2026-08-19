#include "../discord/DiscordClient.h"
#include "../network/socket/Socket.h"
#include "../logging/debug/Debug.h"
#include "../console/Console.h"
#include "../scheduler/Scheduler.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) int __stdcall WSAGetLastError();
}
#endif

namespace Core {
    void sendDiscordApiRequest(void* hOut) {
        NetworkSystem::Socket discordSocket;

        if (!discordSocket.open()) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "Failed to open socket for Discord.");
            return;
        }

        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Socket opened. Connecting to local server...");

        bool connected = false;
        for (int attempt = 1; attempt <= 10; ++attempt) {
            if (discordSocket.connect("127.0.0.1", 8080)) {
                connected = true;
                break;
            }
            // Ждем чуть дольше между попытками (50 мс)
            SchedulerSystem::systemSleep(50);
        }

        if (connected) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Connected! Sending HTTP payload...");

            const char* token = "YourBotTokenHere";

            char requestBuffer[1024] = {0};
            int offset = 0;

            auto appendString = [&requestBuffer, &offset](const char* str) {
                while (str[0] != '\0' && offset < 1023) {
                    requestBuffer[offset++] = str[0];
                    str++;
                }
                requestBuffer[offset] = '\0';
            };

            appendString("GET /api/v10/users/@me HTTP/1.1\r\n");
            appendString("Host: discord.com\r\n");
            appendString("Authorization: Bot ");
            appendString(token);
            appendString("\r\n");
            appendString("User-Agent: DiscordBot (https://github.com/ritasister, 1.0)\r\n");
            appendString("Connection: close\r\n\r\n");

            discordSocket.send(requestBuffer, static_cast<unsigned long>(offset));

            char responseBuffer[1024] = {0};
            long bytesReceived = discordSocket.receive(responseBuffer, sizeof(responseBuffer) - 1);

            if (bytesReceived > 0) {
                DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Response from Server:");
                ConsoleSystem::write(hOut, responseBuffer, static_cast<unsigned long>(bytesReceived));
            } else {
                DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::WARNING, "No response received from server.");
            }
        } else {
#ifdef _WIN32
            int err = WSAGetLastError();
            // Выведет точный код ошибки Windows (например, 10061 - WSAECONNREFUSED)
            char errBuf[64] = {0};
            // Простейший вывод кода ошибки в лог через Logger (если нужно)
#endif
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "Connection to target failed after retries.");
        }
    }
}
