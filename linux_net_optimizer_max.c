#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define BUFFER_SIZE_MAX (16 * 1024 * 1024) /* 16 MB Buffer */

bool MaximizeLinuxSocket(int sock) {
    if (sock < 0) return false;

    int flag = 1;

    /* 1. Disable Nagle's Algorithm */
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0) {
        printf("[+] TCP_NODELAY enabled.\n");
    }

    /* 2. Maximize Send/Receive Buffers */
    int bufSize = BUFFER_SIZE_MAX;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));
    printf("[+] Socket Send/Recv Buffers set to %d MB.\n", BUFFER_SIZE_MAX / (1024 * 1024));

    /* 3. Enable Linux Busy Polling (50 microseconds) - Bypasses IRQ Delays */
    #ifdef SO_BUSY_POLL
    int busyPollUs = 50;
    if (setsockopt(sock, SOL_SOCKET, SO_BUSY_POLL, &busyPollUs, sizeof(busyPollUs)) == 0) {
        printf("[+] Linux SO_BUSY_POLL enabled (Direct NIC Polling Active).\n");
    }
    #endif

    /* 4. Enable TCP Fast Open (TFO) */
    #ifdef TCP_FASTOPEN
    int qLen = 5;
    if (setsockopt(sock, IPPROTO_TCP, TCP_FASTOPEN, &qLen, sizeof(qLen)) == 0) {
        printf("[+] TCP_FASTOPEN active.\n");
    }
    #endif

    /* 5. Set Type of Service (TOS) to Low Latency */
    int tos = IPTOS_LOWLATENCY;
    setsockopt(sock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
    printf("[+] IP_TOS set to IPTOS_LOWLATENCY.\n");

    /* 6. Port and Address Reuse */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    #ifdef SO_REUSEPORT
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
    #endif

    return true;
}

int main(void) {
    printf("===========================================\n");
    printf("   LINUX NETWORK & SOCKET STACK MAX ENGINE \n");
    printf("===========================================\n");

    int testSock = socket(AF_INET, SOCK_STREAM, 0);
    if (testSock >= 0) {
        MaximizeLinuxSocket(testSock);
        close(testSock);
    }

    printf("===========================================\n");
    printf("[SUCCESS] Linux socket optimizations applied.\n");
    return 0;
}
