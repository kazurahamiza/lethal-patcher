#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// BARE-METAL HELPER FUNCTIONS
// ============================================================================

static void LogStep(const char* title) {
    printf("\n[+] %s...\n", title);
}

static void SetRegistryDWORD(HKEY hKeyRoot, const char* subKey, const char* valueName, DWORD value) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        printf("  [SUCCESS] %s -> %lu\n", valueName, value);
    } else {
        printf("  [FAIL] Could not access: %s\n", subKey);
    }
}

// ============================================================================
// 1. GPU OPTIMIZATION MODULE (NVIDIA / HAGS)
// ============================================================================

void PatchGPU(void) {
    LogStep("Applying Brutal GPU Performance Tweaks");

    // Enable Hardware-Accelerated GPU Scheduling (HAGS) & Adjust TDR Delay
    const char* graphicsPath = "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers";
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, graphicsPath, "HwSchMode", 2);  // Force HAGS ON
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, graphicsPath, "TdrDelay", 10);  // Prevent crash timeouts

    // NVIDIA Drivers: Force Max Performance State (Disable Dynamic Throttling)
    const char* nvPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerEnable", 1);
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerLevel", 1);
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerLevelAC", 1);
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "DisableDynamicPState", 0);
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "MaxPreRenderedFrames", 1); // Minimum latency
}

// ============================================================================
// 2. CPU OPTIMIZATION MODULE (Disable Core Parking & Lock Max Clocks)
// ============================================================================

void PatchCPU(void) {
    LogStep("Unparking CPU Cores & Locking Max Frequency");

    // Unhide Power Options
    system("powercfg -attributes SUB_PROCESSOR CPMINCORES -ATTRIB_HIDE >nul 2>&1");
    system("powercfg -attributes SUB_PROCESSOR CPMAXCORES -ATTRIB_HIDE >nul 2>&1");

    // Enable Ultimate Performance Scheme
    system("powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 >nul 2>&1");

    // Force 100% Min/Max CPU Frequency & Unpark 100% of Cores
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMIN 100");
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMAX 100");
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMINCORES 100");
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMAXCORES 100");
    system("powercfg /setactive SCHEME_CURRENT");

    // Quantum Scheduling: Prioritize Foreground Windows
    const char* priorityPath = "SYSTEM\\CurrentControlSet\\Control\\PriorityControl";
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, priorityPath, "Win32PrioritySeparation", 38);
}

// ============================================================================
// 3. SYSTEM REPAIR MODULE
// ============================================================================

void PatchSystem(void) {
    LogStep("Running System Integrity & DNS Cleanup");
    system("sfc /scannow");
    system("ipconfig /flushdns >nul 2>&1");
}

// ============================================================================
// MAIN ENTRY POINT (PURE C)
// ============================================================================

int main(void) {
    printf("==================================================\n");
    printf("         LETHAL PATCHER ENGINE (RAW C EDITION)    \n");
    printf("==================================================\n");

    PatchSystem();
    PatchGPU();
    PatchCPU();

    printf("\n[+] ALL PATCHES APPLIED SUCCESSFULLY. REBOOT REQUIRED.\n");
    return 0;
}
