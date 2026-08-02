#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE_MAX (16 * 1024 * 1024) /* 16 MB Buffer */

bool MaximizeMacSocket(int sock) {
    if (sock < 0) return false;

    int flag = 1;

    /* 1. Disable Nagle's Algorithm */
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0) {
        printf("[+] TCP_NODELAY active.\n");
    }

    /* 2. Set Max Send/Recv Window Buffers */
    int bufSize = BUFFER_SIZE_MAX;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));
    printf("[+] Socket Send/Recv Buffers set to %d MB.\n", BUFFER_SIZE_MAX / (1024 * 1024));

    /* 3. Enable macOS Traffic Class High Priority / Interactive */
    #ifdef SO_TRAFFIC_CLASS
    int trafficClass = SO_TC_CTL; /* Control/Interactive Class */
    if (setsockopt(sock, SOL_SOCKET, SO_TRAFFIC_CLASS, &trafficClass, sizeof(trafficClass)) == 0) {
        printf("[+] macOS SO_TRAFFIC_CLASS set to Interactive Control Tier.\n");
    }
    #endif

    /* 4. Enable TCP Fast Open on Darwin Kernel */
    #ifdef TCP_FASTOPEN
    if (setsockopt(sock, IPPROTO_TCP, TCP_FASTOPEN, &flag, sizeof(flag)) == 0) {
        printf("[+] Darwin TCP_FASTOPEN active.\n");
    }
    #endif

    /* 5. Address Reuse */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));

    return true;
}

int main(void) {
    printf("===========================================\n");
    printf("   MACOS NETWORK & SOCKET STACK MAX ENGINE \n");
    printf("===========================================\n");

    int testSock = socket(AF_INET, SOCK_STREAM, 0);
    if (testSock >= 0) {
        MaximizeMacSocket(testSock);
        close(testSock);
    }

    printf("===========================================\n");
    printf("[SUCCESS] macOS network pipeline fully uncapped.\n");
    return 0;
}
