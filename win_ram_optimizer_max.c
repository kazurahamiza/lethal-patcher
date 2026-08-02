#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma comment(lib, "kernel32.lib")

#define ALLOCATION_SIZE (1024 * 1024 * 64) /* 64 MB Buffer */
#define CACHE_LINE_SIZE 64

/* Enable Lock Pages Privilege required for Large Page Allocations */
bool EnableLockPagesPrivilege(void) {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);
    return true;
}

/* Allocate Direct Memory using Windows 2MB Large Pages */
void* AllocateMaxPerformanceRAMWindows(size_t size) {
    SIZE_T largePageMin = GetLargePageMinimum();
    printf("[i] Minimum System Large Page Size: %zu KB\n", largePageMin / 1024);

    /* Align request size to Large Page granularity */
    size_t alignedSize = (size + largePageMin - 1) & ~(largePageMin - 1);

    EnableLockPagesPrivilege();

    /* Try Huge Page Allocation first */
    void* ptr = VirtualAlloc(
        NULL,
        alignedSize,
        MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
        PAGE_READWRITE
    );

    if (ptr != NULL) {
        printf("[+] Direct 2MB Large Page Allocation Successful at address: %p\n", ptr);
    } else {
        printf("[!] Large Pages failed (Error: %lu). Falling back to Standard Direct Page Commit...\n", GetLastError());
        ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    }

    /* Lock memory pages into physical RAM to prevent page-file paging */
    if (ptr && VirtualLock(ptr, size)) {
        printf("[+] Memory pages locked into physical RAM (Page File Paging Disabled).\n");
    }

    return ptr;
}

int main(void) {
    printf("===========================================\n");
    printf("   WINDOWS ULTRA-LOW LATENCY RAM ENGINE    \n");
    printf("===========================================\n");

    void* ramPtr = AllocateMaxPerformanceRAMWindows(ALLOCATION_SIZE);

    if (ramPtr) {
        printf("[SUCCESS] Unbuffered high-throughput RAM block mapped.\n");
        VirtualFree(ramPtr, 0, MEM_RELEASE);
    }

    printf("===========================================\n");
    return 0;
}
