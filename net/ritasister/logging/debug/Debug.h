#pragma once

namespace DebugSystem {

    // Уровни логирования
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR,
        DEBUG
    };

    /**
     * Класс для управления логированием (консоль + файл bot.log)
     */
    class Logger {
    private:
        static void writeToFile(const char* text, unsigned long size);
        static void appendTwoDigits(char* buf, int& index, int value);

    public:
        // Статический метод для логирования (можно вызывать без создания объекта)
        static void log(void* hOut, LogLevel level, const char* message);
    };

} // namespace DebugSystem
