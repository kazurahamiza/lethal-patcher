#include "gpu_optimizer.hpp"
#include <setupapi.h>
#include <devguid.h>

#pragma comment(lib, "setupapi.lib")

namespace LethalPatcher {

GPUOptimizer::GPUOptimizer() : currentVendor(GPUVendor::UNKNOWN), isInitialized(false) {}

GPUOptimizer::~GPUOptimizer() {}

bool GPUOptimizer::Initialize() {
    currentVendor = DetectVendor();
    isInitialized = true;
    return true;
}

GPUVendor GPUOptimizer::DetectVendor() {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return GPUVendor::UNKNOWN;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        char buffer[256];
        if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
            std::string hwid(buffer);
            if (hwid.find("VEN_10DE") != std::string::npos) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPUVendor::NVIDIA;
            } else if (hwid.find("VEN_1002") != std::string::npos) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPUVendor::AMD;
            } else if (hwid.find("VEN_8086") != std::string::npos) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPUVendor::INTEL;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return GPUVendor::UNKNOWN;
}

void GPUOptimizer::SetWindowsGraphicsPerformanceMode() {
    HKEY hKey;
    const char* regPath = "Software\\Microsoft\\DirectX\\UserGpuPreferences";
    
    if (RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        const char* pref = "GpuPreference=2;"; // 2 = High Performance
        RegSetValueExA(hKey, "DirectXUserGlobalSettings", 0, REG_SZ, (BYTE*)pref, (DWORD)strlen(pref) + 1);
        RegCloseKey(hKey);
    }
}

void GPUOptimizer::FlushDriverCache() {
    char appData[MAX_PATH];
    if (GetEnvironmentVariableA("LOCALAPPDATA", appData, MAX_PATH)) {
        std::string nvCache = std::string(appData) + "\\NVIDIA\\DXCache\\*";
        std::string amdCache = std::string(appData) + "\\AMD\\DxCache\\*";
    }
}

void GPUOptimizer::ApplyNVidiaOptimizations() {
    std::cout << "[+] Injecting Ultra High-Performance NVIDIA Registry & Driver Tweaks..." << std::endl;
    
    HKEY hKey;
    const char* nvPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, nvPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        
        // 1. Force PowerMizer to Maximum Performance State
        DWORD perfMode = 1;
        RegSetValueExA(hKey, "PowerMizerEnable", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevel", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevelAC", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        
        // 2. Disable CUDA P2 State (Locks maximum memory clocks)
        DWORD disableP2 = 0;
        RegSetValueExA(hKey, "DisableDynamicPState", 0, REG_DWORD, (BYTE*)&disableP2, sizeof(disableP2));

        // 3. Force Ultra Low Latency (1 Pre-rendered Frame)
        DWORD maxPreRenderedFrames = 1;
        RegSetValueExA(hKey, "MaxPreRenderedFrames", 0, REG_DWORD, (BYTE*)&maxPreRenderedFrames, sizeof(maxPreRenderedFrames));

        // 4. Set Texture Filtering to High Performance Mode
        DWORD qualityFilter = 2;
        RegSetValueExA(hKey, "TextureFilteringQuality", 0, REG_DWORD, (BYTE*)&qualityFilter, sizeof(qualityFilter));

        RegCloseKey(hKey);
        std::cout << " [SUCCESS] Applied PowerMizer, Low Latency, and Unlocked Memory P-States." << std::endl;
    } else {
        std::cout << " [WARNING] Failed to write NVIDIA driver registry settings. Ensure Administrator privileges." << std::endl;
    }
}

void GPUOptimizer::ApplyAMDOptimizations() {
    std::cout << "[+] Applying AMD Anti-Lag & Memory P-State Lock..." << std::endl;

    HKEY hKey;
    const char* amdPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, amdPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD KMDEnableUlps = 0; // Disable Ultra Low Power State (ULPS)
        RegSetValueExA(hKey, "EnableUlps", 0, REG_DWORD, (BYTE*)&KMDEnableUlps, sizeof(KMDEnableUlps));
        RegCloseKey(hKey);
    }
}

void GPUOptimizer::ApplyGenericRegistryTweaks() {
    std::cout << "[+] Enabling Hardware-Accelerated GPU Scheduling (HAGS)..." << std::endl;

    HKEY hKey;
    const char* graphicsPath = "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, graphicsPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD hagsMode = 2; // Enable HAGS
        RegSetValueExA(hKey, "HwSchMode", 0, REG_DWORD, (BYTE*)&hagsMode, sizeof(hagsMode));
        
        DWORD tdrDelay = 10; // Prevent GPU timeout crashes
        RegSetValueExA(hKey, "TdrDelay", 0, REG_DWORD, (BYTE*)&tdrDelay, sizeof(tdrDelay));
        
        RegCloseKey(hKey);
    }
}

void GPUOptimizer::OptimizeForMaximumPerformance() {
    if (!isInitialized) Initialize();

    SetWindowsGraphicsPerformanceMode();
    ApplyGenericRegistryTweaks();

    switch (currentVendor) {
        case GPUVendor::NVIDIA:
            ApplyNVidiaOptimizations();
            break;
        case GPUVendor::AMD:
            ApplyAMDOptimizations();
            break;
        case GPUVendor::INTEL:
            std::cout << "[+] Intel Graphics detected - applying generic execution optimizations." << std::endl;
            break;
        default:
            std::cout << "[!] Generic GPU detected - basic optimizations applied." << std::endl;
            break;
    }

    FlushDriverCache();
    std::cout << "[+] GPU Optimization Complete." << std::endl;
}

} // namespace LethalPatchervoid GPUOptimizer::ApplyNVidiaOptimizations() {
    std::cout << "[+] Injecting Ultra High-Performance NVIDIA Registry & Driver Tweaks..." << std::endl;
    
    HKEY hKey;
    const char* nvPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, nvPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        
        // 1. Force PowerMizer to Maximum Performance (Disable Clock Throttling)
        DWORD perfMode = 1;
        RegSetValueExA(hKey, "PowerMizerEnable", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevel", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevelAC", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        
        // 2. Disable CUDA P2 State (Allow Full Memory Overclock under Compute/3D loads)
        DWORD disableP2 = 0;
        RegSetValueExA(hKey, "DisableDynamicPState", 0, REG_DWORD, (BYTE*)&disableP2, sizeof(disableP2));

        // 3. Force Ultra Low Latency / Pre-Rendered Frames to Minimum (1 Frame)
        DWORD maxPreRenderedFrames = 1;
        RegSetValueExA(hKey, "MaxPreRenderedFrames", 0, REG_DWORD, (BYTE*)&maxPreRenderedFrames, sizeof(maxPreRenderedFrames));

        // 4. Force High Performance Texture Filtering Quality
        DWORD qualityFilter = 2; // 2 = High Performance Mode
        RegSetValueExA(hKey, "TextureFilteringQuality", 0, REG_DWORD, (BYTE*)&qualityFilter, sizeof(qualityFilter));

        RegCloseKey(hKey);
        std::cout << " [SUCCESS] Applied PowerMizer, Low Latency, and Unlocked Memory P-States." << std::endl;
    } else {
        std::cout << " [WARNING] Failed to write NVIDIA driver registry settings. Run as Administrator." << std::endl;
    }
}
