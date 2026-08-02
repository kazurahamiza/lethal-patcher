#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <stdio.h>

#define ALLOCATION_SIZE (1024 * 1024 * 64) /* 64 MB */

bool AllocateMetalUnifiedMemoryBuffer(void) {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        printf("[-] Metal API unsupported on device.\n");
        return false;
    }

    printf("[+] Active Metal GPU: %s\n", [[device name] UTF8String]);

    /* Mark buffer as Shared Storage Mode (Zero-Copy Unified RAM Access) */
    id<MTLBuffer> unifiedBuffer = [device newBufferWithLength:ALLOCATION_SIZE 
                                                      options:MTLResourceStorageModeShared];

    if (unifiedBuffer) {
        void* hostPointer = [unifiedBuffer contents];
        printf("[+] Zero-Copy Unified VRAM Buffer Created at CPU/GPU Pointer: %p\n", hostPointer);
        return true;
    }

    return false;
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        printf("===========================================\n");
        printf("   MACOS METAL UNIFIED VRAM MAX ENGINE     \n");
        printf("===========================================\n");

        AllocateMetalUnifiedMemoryBuffer();

        printf("===========================================\n");
    }
    return 0;
}
