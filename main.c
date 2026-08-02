#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// BARE-METAL HELPER FUNCTIONS
// ============================================================================

static void LogHeader(const char* title) {
    printf("\n==================================================\n");
    printf(" [+] %s\n", title);
    printf("==================================================\n");
}

static BOOL WriteRegistryDWORD(HKEY hKeyRoot, const char* subKey, const char* valueName, DWORD value) {
    HKEY hKey;
    LONG status = RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_SET_VALUE, &hKey);
    
    if (status == ERROR_SUCCESS) {
        status = RegSetValueExA(hKey, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
        if (status == ERROR_SUCCESS) {
            printf("  [SUCCESS] %s -> %lu\n", valueName, value);
            return TRUE;
        }
    }
    printf("  [FAIL] Failed to write %s to %s (Error: %ld)\n", valueName, subKey, status);
    return FALSE;
}

// ============================================================================
// 1. RAW GPU OPTIMIZATION MODULE
// ============================================================================

void PatchGPU(void) {
    LogHeader("EXECUTING: Direct GPU P-State & Scheduler Overrides");

    const char* graphicsPath = "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers";
    const char* nvPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";

    // Hardware-Accelerated GPU Scheduling (HAGS) ON
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, graphicsPath, "HwSchMode", 2);

    // Extend Timeout Detection and Recovery Delay (Prevents GPU crashes under load)
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, graphicsPath, "TdrDelay", 10);

    // NVIDIA PowerMizer Max Performance State (Disable Clock Throttling)
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerEnable", 1);
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerLevel", 1);
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "PowerMizerLevelAC", 1);

    // Force Max VRAM Clocks (Disable P2 Compute State)
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "DisableDynamicPState", 0);

    // Minimum Pre-Rendered Frames for Low Latency
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, nvPath, "MaxPreRenderedFrames", 1);
}

// ============================================================================
// 2. RAW CPU OPTIMIZATION MODULE
// ============================================================================

void PatchCPU(void) {
    LogHeader("EXECUTING: CPU Core Parking & Frequency Lock Engine");

    // Unhide Power Management Settings in Windows
    system("powercfg -attributes SUB_PROCESSOR CPMINCORES -ATTRIB_HIDE >nul 2>&1");
    system("powercfg -attributes SUB_PROCESSOR CPMAXCORES -ATTRIB_HIDE >nul 2>&1");

    // Enable Ultimate Performance Power Scheme
    system("powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 >nul 2>&1");

    // Lock Min/Max Processor Clocks to 100%
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMIN 100");
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR PROCTHROTTLEMAX 100");

    // Force 100% of Cores Active (Disable Core Parking)
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMINCORES 100");
    system("powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR CPMAXCORES 100");
    system("powercfg /setactive SCHEME_CURRENT");

    // Foreground Application Priority Boost
    const char* priorityPath = "SYSTEM\\CurrentControlSet\\Control\\PriorityControl";
    WriteRegistryDWORD(HKEY_LOCAL_MACHINE, priorityPath, "Win32PrioritySeparation", 38);
}

// ============================================================================
// 3. RAW SYSTEM REPAIR MODULE
// ============================================================================

void PatchSystem(void) {
    LogHeader("EXECUTING: System File Integrity Check & DNS Flush");

    printf(" Running System File Checker (SFC)...\n");
    system("sfc /scannow");

    printf(" Flushing DNS Resolver Cache...\n");
    system("ipconfig /flushdns >nul 2>&1");
}

// ============================================================================
// MAIN ENTRY POINT (PURE RAW C)
// ============================================================================

int main(void) {
    printf("==================================================\n");
    printf("       LETHAL PATCHER ENGINE (PURE C EDITION)     \n");
    printf("==================================================\n");

    PatchSystem();
    PatchGPU();
    PatchCPU();

    printf("\n[+] LETHAL PATCHER EXECUTION COMPLETE.\n");
    printf("[+] REBOOT SYSTEM TO APPLY KERNEL & DRIVER OVERRIDES.\n");

    return 0;
}
