#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "setupapi.lib")

typedef enum {
    GPU_VENDOR_UNKNOWN = 0,
    GPU_VENDOR_NVIDIA,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL
} GPUVendor;

typedef struct {
    GPUVendor currentVendor;
    bool isInitialized;
} GPUOptimizer;

/* Global or instance initialization */
void GPUOptimizer_Init(GPUOptimizer* optimizer) {
    if (optimizer) {
        optimizer->currentVendor = GPU_VENDOR_UNKNOWN;
        optimizer->isInitialized = false;
    }
}

void GPUOptimizer_Cleanup(GPUOptimizer* optimizer) {
    if (optimizer) {
        optimizer->isInitialized = false;
    }
}

GPUVendor GPUOptimizer_DetectVendor(GPUOptimizer* optimizer) {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return GPU_VENDOR_UNKNOWN;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        char buffer[256];
        if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), NULL)) {
            if (strstr(buffer, "VEN_10DE") != NULL) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPU_VENDOR_NVIDIA;
            } else if (strstr(buffer, "VEN_1002") != NULL) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPU_VENDOR_AMD;
            } else if (strstr(buffer, "VEN_8086") != NULL) {
                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                return GPU_VENDOR_INTEL;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return GPU_VENDOR_UNKNOWN;
}

bool GPUOptimizer_Initialize(GPUOptimizer* optimizer) {
    if (!optimizer) return false;

    optimizer->currentVendor = GPUOptimizer_DetectVendor(optimizer);
    optimizer->isInitialized = true;
    return true;
}
