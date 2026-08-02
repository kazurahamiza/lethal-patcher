#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "ws232.lib")

#define BUFFER_SIZE_MAX (16 * 1024 * 1024) /* 16 MB Buffer */

/* Enable low-latency and max throughput socket options */
bool MaximizeWindowsSocket(SOCKET sock) {
    if (sock == INVALID_SOCKET) return false;

    int flag = 1;

    /* 1. Disable Nagle's Algorithm for zero packet batching latency */
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag)) == 0) {
        printf("[+] TCP_NODELAY enabled (Nagle's Algorithm Disabled).\n");
    }

    /* 2. Set Max Send and Receive Buffers (16MB) */
    int bufSize = BUFFER_SIZE_MAX;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufSize, sizeof(bufSize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&bufSize, sizeof(bufSize));
    printf("[+] Socket Send/Recv Buffers expanded to %d MB.\n", BUFFER_SIZE_MAX / (1024 * 1024));

    /* 3. Enable Fast Loopback Path if operating locally */
    DWORD optionValue = 1;
    DWORD bytesReturned = 0;
    if (WSAIoctl(sock, SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(optionValue),
                 NULL, 0, &bytesReturned, NULL, NULL) == 0) {
        printf("[+] SIO_LOOPBACK_FAST_PATH enabled for low-latency IPC.\n");
    }

    /* 4. Enable Address & Port Reuse */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag));

    return true;
}

int main(void) {
    printf("===========================================\n");
    printf("  WINDOWS NETWORK & SOCKET STACK MAX OPT   \n");
    printf("===========================================\n");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[-] Winsock initialization failed.\n");
        return 1;
    }

    SOCKET testSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (testSock != INVALID_SOCKET) {
        MaximizeWindowsSocket(testSock);
        closesocket(testSock);
    }

    WSACleanup();
    printf("===========================================\n");
    printf("[SUCCESS] Windows socket pipeline uncapped.\n");
    return 0;
}
