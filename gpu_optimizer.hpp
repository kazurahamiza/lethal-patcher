#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

namespace LethalPatcher {

enum class GPUVendor {
    NVIDIA,
    AMD,
    INTEL,
    UNKNOWN
};

class GPUOptimizer {
public:
    GPUOptimizer();
    ~GPUOptimizer();

    bool Initialize();
    GPUVendor DetectVendor();
    void OptimizeForMaximumPerformance();
    void ApplyNVidiaOptimizations();
    void ApplyAMDOptimizations();
    void ApplyGenericRegistryTweaks();

private:
    GPUVendor currentVendor;
    bool isInitialized;

    void SetWindowsGraphicsPerformanceMode();
    void FlushDriverCache();
};

} // namespace LethalPatcher
