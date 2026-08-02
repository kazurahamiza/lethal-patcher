#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/vm_map.h>

#define ALLOCATION_SIZE (1024 * 1024 * 64) /* 64 MB */
#define CACHE_LINE_SIZE 128 /* Optimized for Apple Silicon L1 cache line size */

void* AllocateMaxPerformanceRAMMac(size_t size) {
    void* ptr = NULL;

    /* Use Mach kernel vm_allocate for direct virtual memory mapping */
    kern_return_t kr = vm_allocate(mach_task_self(), (vm_address_t*)&ptr, size, VM_FLAGS_ANYWHERE);

    if (kr != KERN_SUCCESS) {
        printf("[!] Mach vm_allocate failed. Falling back to POSIX cache-aligned allocation...\n");
        if (posix_memalign(&ptr, CACHE_LINE_SIZE, size) != 0) {
            return NULL;
        }
    } else {
        printf("[+] Mach Kernel Direct Virtual Memory Allocated at %p.\n", ptr);
    }

    /* Lock virtual pages into physical Unified Memory RAM */
    if (mlock(ptr, size) == 0) {
        printf("[+] Memory pages locked into macOS Unified RAM (Paging Bypass Active).\n");
    }

    /* Advise Mach VM subsystem for immediate low-latency read/write access */
    madvise(ptr, size, MADV_WILLNEED);

    return ptr;
}

int main(void) {
    printf("===========================================\n");
    printf("   MACOS UNIFIED MEMORY / RAM MAX ENGINE   \n");
    printf("===========================================\n");

    void* ramPtr = AllocateMaxPerformanceRAMMac(ALLOCATION_SIZE);

    if (ramPtr) {
        printf("[SUCCESS] Low-latency macOS RAM pipeline initialized.\n");
        munmap(ramPtr, ALLOCATION_SIZE);
    }

    printf("===========================================\n");
    return 0;
}
