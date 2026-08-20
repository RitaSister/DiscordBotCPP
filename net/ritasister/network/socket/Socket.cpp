#include "Socket.h"

#ifdef _WIN32
extern "C" {
    __declspec(dllimport) unsigned long long __stdcall socket(int af, int type, int protocol);
    __declspec(dllimport) int __stdcall connect(unsigned long long s, const void* name, int namelen);
    __declspec(dllimport) int __stdcall send(unsigned long long s, const char* buf, int len, int flags);
    __declspec(dllimport) int __stdcall recv(unsigned long long s, char* buf, int len, int flags);
    __declspec(dllimport) int __stdcall closesocket(unsigned long long s);
    __declspec(dllimport) unsigned long __stdcall inet_addr(const char* cp);
    __declspec(dllimport) unsigned short __stdcall htons(unsigned short hostshort);
    __declspec(dllimport) int __stdcall setsockopt(unsigned long long s, int level, int optname, const char* optval, int optlen);
    __declspec(dllimport) int __stdcall getaddrinfo(const char* nodename, const char* servname, const void* hints, void** res);
    __declspec(dllimport) int __stdcall freeaddrinfo(void* ai);
}
#endif

namespace NetworkSystem {

    Socket::Socket() : socketHandle(nullptr), connected(false) {}

    Socket::~Socket() {
        close();
    }

    bool Socket::open() {
        if (socketHandle != nullptr) return true;

#ifdef _WIN32
        unsigned long long s = socket(2, 1, 6);
        if (s == ~0ULL) {
            return false;
        }

        unsigned long timeoutMs = 3000;
        setsockopt(s, 0xFFFF, 0x1006, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
        setsockopt(s, 0xFFFF, 0x1005, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

        socketHandle = reinterpret_cast<void*>(s);
        return true;
#else
        return false;
#endif
    }

    bool Socket::connect(const char* ip, unsigned short port) {
        if (!socketHandle) return false;

#ifdef _WIN32
        unsigned long long s = reinterpret_cast<unsigned long long>(socketHandle);

        char addr[16] = {0};
        *reinterpret_cast<unsigned short*>(addr) = 2;
        *reinterpret_cast<unsigned short*>(addr + 2) = htons(port);
        *reinterpret_cast<unsigned long*>(addr + 4) = inet_addr(ip);

        int result = ::connect(s, addr, 16);
        connected = (result == 0);
        return connected;
#else
        return false;
#endif
    }

    bool Socket::send(const char* data, unsigned long size) {
        if (!connected || !socketHandle) return false;

#ifdef _WIN32
        unsigned long long s = reinterpret_cast<unsigned long long>(socketHandle);
        int sent = ::send(s, data, static_cast<int>(size), 0);
        return sent != -1;
#else
        return false;
#endif
    }

    long Socket::receive(char* buffer, unsigned long size) {
        if (!connected || !socketHandle) return -1;

#ifdef _WIN32
        unsigned long long s = reinterpret_cast<unsigned long long>(socketHandle);
        return static_cast<long>(::recv(s, buffer, static_cast<int>(size), 0));
#else
        return -1;
#endif
    }

    void Socket::close() {
        if (socketHandle) {
#ifdef _WIN32
            unsigned long long s = reinterpret_cast<unsigned long long>(socketHandle);
            closesocket(s);
#endif
            socketHandle = nullptr;
            connected = false;
        }
    }

    bool Socket::isConnected() const {
        return connected;
    }

    // Автономный DNS-резолвер
    bool resolveDomainToIp(const char* domain, char* ipOutputBuffer, unsigned long bufferSize) {
#ifdef _WIN32
        void* resultList = nullptr;

        if (getaddrinfo(domain, "443", nullptr, &resultList) != 0 || !resultList) {
            return false;
        }

        unsigned char* addrStruct = *reinterpret_cast<unsigned char**>(reinterpret_cast<char*>(resultList) + 24);
        if (addrStruct) {
            unsigned long ipVal = *reinterpret_cast<unsigned long*>(addrStruct + 4);

            unsigned char* bytes = reinterpret_cast<unsigned char*>(&ipVal);
            int offset = 0;

            for (int i = 0; i < 4; ++i) {
                unsigned char b = bytes[i];
                if (b >= 100) {
                    ipOutputBuffer[offset++] = '0' + (b / 100);
                    b %= 100;
                    ipOutputBuffer[offset++] = '0' + (b / 10);
                    ipOutputBuffer[offset++] = '0' + (b % 10);
                } else if (b >= 10) {
                    ipOutputBuffer[offset++] = '0' + (b / 10);
                    ipOutputBuffer[offset++] = '0' + (b % 10);
                } else {
                    ipOutputBuffer[offset++] = '0' + b;
                }
                if (i < 3) ipOutputBuffer[offset++] = '.';
            }
            ipOutputBuffer[offset] = '\0';

            freeaddrinfo(resultList);
            return true;
        }

        freeaddrinfo(resultList);
#endif
        return false;
    }

} // namespace NetworkName
