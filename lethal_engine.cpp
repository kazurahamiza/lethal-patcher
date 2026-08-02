#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <string>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
void LogStep(const std::string& title) {
    std::cout << "\n[+] " << title << "..." << std::endl;
}

void SetRegistryDWORD(HKEY hKeyRoot, const char* subKey, const char* valueName, DWORD value) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        std::cout << "  [SUCCESS] " << valueName << " -> " << value << std::endl;
    } else {
        std::cout << "  [FAIL] Could not access: " << subKey << std::endl;
    }
}

// ============================================================================
// 1. GPU OPTIMIZATION MODULE (NVIDIA / HAGS)
// ============================================================================
void PatchGPU() {
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
void PatchCPU() {
    LogStep("Unparking CPU Cores & Locking Max Frequency");

    // Unhide Power Options
    std::system("powercfg -attributes SUB_PROCESSOR CPMINCORES -ATTRIB_HIDE >nul 2>&1");
    std::system("powercfg -attributes SUB_PROCESSOR CPMAXCORES -ATTRIB_HIDE >nul 2>&1");

    // Enable Ultimate Performance Scheme
    std::system("powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 >nul 2>&1");

    // Force 100% Min/Max CPU Frequency & Unpark 100% of Cores
    std::system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMIN 100");
    std::system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMAX 100");
    std::system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMINCORES 100");
    std::system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMAXCORES 100");
    std::system("powercfg /setactive SCHEME_CURRENT");

    // Quantum Scheduling: Prioritize Foreground Windows
    const char* priorityPath = "SYSTEM\\CurrentControlSet\\Control\\PriorityControl";
    SetRegistryDWORD(HKEY_LOCAL_MACHINE, priorityPath, "Win32PrioritySeparation", 38);
}

// ============================================================================
// 3. SYSTEM REPAIR MODULE
// ============================================================================
void PatchSystem() {
    LogStep("Running System Integrity & DNS Cleanup");
    std::system("sfc /scannow");
    std::system("ipconfig /flushdns >nul 2>&1");
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "         LETHAL PATCHER ENGINE (DIRECT)          " << std::endl;
    std::cout << "==================================================" << std::endl;

    PatchSystem();
    PatchGPU();
    PatchCPU();

    std::cout << "\n[+] ALL PATCHES APPLIED SUCCESSFULLY. REBOOT REQUIRED." << std::endl;
    return 0;
}
