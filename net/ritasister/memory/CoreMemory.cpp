#ifdef _WIN32
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void* hHeap, unsigned long dwFlags, unsigned long long dwBytes);
__declspec(dllimport) int __stdcall HeapFree(void* hHeap, unsigned long dwFlags, void* lpMem);
}
#endif

extern "C" {
void* memset(void* dest, int c, unsigned long long n) {
    unsigned char* p = static_cast<unsigned char*>(dest);
    while (n--) {
        *p++ = static_cast<unsigned char>(c);
    }
    return dest;
}

void* memcpy(void* dest, const void* src, unsigned long long n) {
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

unsigned long long strlen(const char* str) {
    const char* s = str;
    while (*s) {
        s++;
    }
    return static_cast<unsigned long long>(s - str);
}

void __cdecl ___chkstk_ms() {}
void __cdecl _chkstk_ms() {}

int __mingw_printf(const char* format, ...) { return 0; }
int printf(const char* format, ...) { return 0; }
int puts(const char* str) { return 0; }
int putchar(int c) { return c; }
}

// Глобальные операторы new / delete через WinAPI Heap
void* operator new(unsigned long long size) {
#ifdef _WIN32
    return HeapAlloc(GetProcessHeap(), 0, size ? size : 1);
#else
    return nullptr;
#endif
}

void operator delete(void* ptr, unsigned long long) noexcept {
#ifdef _WIN32
    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
#endif
}

void operator delete(void* ptr) noexcept {
#ifdef _WIN32
    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
#endif
}
