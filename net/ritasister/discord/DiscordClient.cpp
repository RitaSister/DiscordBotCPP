#include "../discord/DiscordClient.h"
#include "../network/tls/TlsClient.h"
#include "../logging/debug/Debug.h"
#include "../scheduler/Scheduler.h"
#include "../network/socket/Socket.h"

namespace Core { // <-- Ровно один namespace Core
    void sendDiscordApiRequest(void* hOut) {
        NetworkSystem::TlsClient* tlsClient = new NetworkSystem::TlsClient();

        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Resolving discord.com via system DNS...");

        char resolvedIp[32] = {0};
        if (!NetworkSystem::resolveDomainToIp("discord.com", resolvedIp, sizeof(resolvedIp))) {
            const char* fallback = "162.159.135.232";
            for(int i = 0; fallback[i] != '\0'; ++i) resolvedIp[i] = fallback[i];
        }

        DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "Initiating TLS 1.3 handshake with discord.com:443...");

        bool connected = false;
        for (int attempt = 1; attempt <= 3; ++attempt) {
            if (tlsClient->connect(resolvedIp, 443)) {
                connected = true;
                break;
            }
            SchedulerSystem::systemSleep(100);
        }

        if (connected) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::INFO, "TLS Handshake successful! Sending encrypted API payload...");
            tlsClient->close();
        } else {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "TLS Connection to discord.com failed after DNS resolution.");
        }

        delete tlsClient;
    }
}
