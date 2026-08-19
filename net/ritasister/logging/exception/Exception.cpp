#include "Exception.h"
#include "../../network/socket/Socket.h"
#include "../../logging/debug/Debug.h"

namespace NetworkSystem {
    class Socket;

    // Функция открытия сокета, возвращающая кастомный статус/ошибку
    Core::ErrorSystem::Exception openSocketSafe(Socket& clientSocket, void* hOut) {
        if (!clientSocket.open()) {
            DebugSystem::Logger::log(hOut, DebugSystem::LogLevel::ERROR, "Failed to open socket.");
            return Core::ErrorSystem::Exception(
                Core::ErrorSystem::ErrorCode::SocketOpenFailed, 
                "Socket open operation failed"
            );
        }
        return Core::ErrorSystem::Exception(
            Core::ErrorSystem::ErrorCode::OK, 
            "Success"
        );
    }

} // namespace NetworkSystem
