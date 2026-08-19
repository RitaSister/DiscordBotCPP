#include "Scheduler.h"

#ifdef _WIN32
extern "C" {
__declspec(dllimport) unsigned long long __stdcall GetTickCount64();
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
}
#elif __linux__
#include <unistd.h>
#include <time.h>
#endif

namespace SchedulerSystem {

    unsigned long long getSystemTimeMs() {
#ifdef _WIN32
        return GetTickCount64();
#elif __linux__
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000ULL) + (ts.tv_nsec / 1000000ULL);
#else
        return 0;
#endif
    }

    void systemSleep(unsigned long ms) {
#ifdef _WIN32
        Sleep(ms);
#elif __linux__
        usleep(ms * 1000);
#endif
    }

} // namespace SchedulerSystem
