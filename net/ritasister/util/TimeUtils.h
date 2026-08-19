#pragma once

namespace Utils {

    class TimeUtils {
    public:
        // Заполняет переданный буфер отформатированной строкой времени [HH:MM:SS]
        // Возвращает количество записанных байт
        static int getFormattedTime(char* buffer, unsigned long maxSize);
    };

} // namespace Utils
