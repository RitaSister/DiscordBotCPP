#pragma once

namespace DebugSystem {
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR_LVL,
        DEBUG
    };

    void log(void* hOut, LogLevel level, const char* message);
}
