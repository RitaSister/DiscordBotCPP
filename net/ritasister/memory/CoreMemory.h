#pragma once

extern "C" {

void* memset(void* dest, int c, unsigned long long n);
void* memcpy(void* dest, const void* src, unsigned long long n);
unsigned long long strlen(const char* str);

// ВАЖНО: именно два `_`
void __cdecl _chkstk_ms();

int _mingw_printf(const char* format, ...);
int printf(const char* format, ...);
int puts(const char* str);
int putchar(int c);

}