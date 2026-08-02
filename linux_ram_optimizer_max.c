#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>

#define ALLOCATION_SIZE (1024 * 1024 * 64) /* 64 MB */
#define CACHE_LINE_SIZE 64

/* Map memory using Linux Kernel HugeTLB (2MB Hardware Pages) */
void* AllocateHugeTLBMemoryLinux(size_t size) {
    void* ptr = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
        -1,
        0
    );

    if (ptr == MAP_FAILED) {
        printf("[!] MAP_HUGETLB allocation failed. Ensure hugetlb is configured in /proc/sys/vm/nr_hugepages.\n");
        printf("[*] Falling back to standard aligned memory mapping...\n");

        if (posix_memalign(&ptr, CACHE_LINE_SIZE, size) != 0) {
            printf("[-] Memory allocation failed completely.\n");
            return NULL;
        }
        printf("[+] Allocated %zu bytes with %d-byte L1 Cache Line Alignment.\n", size, CACHE_LINE_SIZE);
    } else {
        printf("[+] Mapped %zu bytes using Hardware HugeTLB Pages at address %p.\n", size, ptr);
    }

    /* Prevent kernel swapping to disk */
    if (mlock(ptr, size) == 0) {
        printf("[+] Memory region locked into RAM (mlock active).\n");
    } else {
        printf("[-] mlock failed. Run as root or update ulimit -l.\n");
    }

    /* Advise kernel that this memory will be accessed aggressively */
    madvise(ptr, size, MADV_WILLNEED | MADV_HUGEPAGE);

    return ptr;
}

int main(void) {
    printf("===========================================\n");
    printf("   LINUX ULTRA-LOW LATENCY RAM ENGINE     \n");
    printf("===========================================\n");

    void* ramPtr = AllocateHugeTLBMemoryLinux(ALLOCATION_SIZE);

    if (ramPtr) {
        printf("[SUCCESS] Uncapped Linux RAM block active.\n");
        munmap(ramPtr, ALLOCATION_SIZE);
    }

    printf("===========================================\n");
    return 0;
}
