extern "C" {
// Заглушка для сборщика мусора/инициализатора компилятора MinGW
void _main() {}

__declspec(dllimport) void* __stdcall CreateFileA(
    const char* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode,
    void* lpSecurityAttributes, unsigned long dwCreationDisposition,
    unsigned long dwFlagsAndAttributes, void* hTemplateFile
);
__declspec(dllimport) int __stdcall ReadFile(
    void* hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead,
    unsigned long* lpNumberOfBytesRead, void* lpOverlapped
);
__declspec(dllimport) int __stdcall WriteFile(
    void* hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite,
    unsigned long* lpNumberOfBytesWritten, void* lpOverlapped
);
__declspec(dllimport) void __stdcall ExitProcess(unsigned int uExitCode);
}

// Объявляем консольную точку входа Windows напрямую
extern "C" void mainCRTStartup() {
    void* hOut = CreateFileA("CONOUT$", 0x40000000, 2, nullptr, 3, 0, nullptr);
    void* hIn  = CreateFileA("CONIN$",  0x80000000, 1, nullptr, 3, 0, nullptr);

    if (hOut == reinterpret_cast<void *>(-1) || hIn == reinterpret_cast<void *>(-1)) {
        ExitProcess(1);
    }

    const char* msg = "Loop started. Type 'q' to exit.\n";
    unsigned long written = 0;
    WriteFile(hOut, msg, 32, &written, nullptr);

    char buffer[16];

    while (true) {
        unsigned long read = 0;
        if (ReadFile(hIn, buffer, sizeof(buffer) - 1, &read, nullptr) && read > 0) {
            if (buffer[0] == 'q') {
                break;
            }

            WriteFile(hOut, "Echo: ", 6, &written, nullptr);
            WriteFile(hOut, buffer, read, &written, nullptr);
        }
    }

    ExitProcess(0);
}
