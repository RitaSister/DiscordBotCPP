#include "TimeUtils.h"

#ifdef _WIN32
extern "C" {
struct SYSTEMTIME {
    unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};
__declspec(dllimport) void __stdcall GetLocalTime(SYSTEMTIME* lpSystemTime);
}
#elif __linux__
#include <time.h>
#endif

namespace Utils {

    // Вспомогательная функция для добавления ведущего нуля
    static void appendTwoDigits(char* buf, int& index, int value) {
        buf[index++] = '0' + (value / 10);
        buf[index++] = '0' + (value % 10);
    }

    int TimeUtils::getFormattedTime(char* buffer, unsigned long maxSize) {
        if (!buffer || maxSize < 10) return 0;

        int hour = 0, minute = 0, second = 0;

#ifdef _WIN32
        SYSTEMTIME st;
        GetLocalTime(&st);
        hour = st.wHour;
        minute = st.wMinute;
        second = st.wSecond;
#elif __linux__
        time_t rawtime;
        time(&rawtime);
        struct tm* timeinfo = localtime(&rawtime);
        if (timeinfo) {
            hour = timeinfo->tm_hour;
            minute = timeinfo->tm_min;
            second = timeinfo->tm_sec;
        }
#endif

        int idx = 0;
        buffer[idx++] = '[';
        appendTwoDigits(buffer, idx, hour);
        buffer[idx++] = ':';
        appendTwoDigits(buffer, idx, minute);
        buffer[idx++] = ':';
        appendTwoDigits(buffer, idx, second);
        buffer[idx++] = ']';
        buffer[idx++] = ' ';
        buffer[idx] = '\0';

        return idx; // Возвращает размер таймстампа (обычно 10 байт)
    }

} // namespace Utils
