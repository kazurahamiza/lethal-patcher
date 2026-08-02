#ifndef GPU_OPTIMIZER_H
#define GPU_OPTIMIZER_H

#include <windows.h>
#include <stdbool.h>

typedef enum {
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_UNKNOWN
} GPUVendor;

typedef struct {
    GPUVendor currentVendor;
    bool isInitialized;
} GPUOptimizer;

/* Function Declarations */
void GPUOptimizer_Init(GPUOptimizer* optimizer);
void GPUOptimizer_Cleanup(GPUOptimizer* optimizer);

bool GPUOptimizer_Initialize(GPUOptimizer* optimizer);
GPUVendor GPUOptimizer_DetectVendor(GPUOptimizer* optimizer);

void GPUOptimizer_OptimizeForMaximumPerformance(GPUOptimizer* optimizer);
void GPUOptimizer_ApplyNvidiaOptimizations(GPUOptimizer* optimizer);
void GPUOptimizer_ApplyAMDOptimizations(GPUOptimizer* optimizer);
void GPUOptimizer_ApplyGenericRegistryTweaks(GPUOptimizer* optimizer);

#endif /* GPU_OPTIMIZER_H */
