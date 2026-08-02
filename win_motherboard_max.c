#include <windows.h>
#include <powrprof.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "PowrProf.lib")

/* GUIDs for Windows High Performance & Power Settings */
static const GUID GUID_MAX_PERFORMANCE = 
    { 0x8c5e7fda, 0xe8bf, 0x4a96, { 0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8e, 0x63, 0x5c } };

static const GUID GUID_PCIEXPRESS_SETTINGS = 
    { 0x501a4d13, 0x42af, 0x4429, { 0x9f, 0xd1, 0xa8, 0x21, 0x8c, 0x26, 0x8e, 0x20 } };

static const GUID GUID_ASPM_SETTING = 
    { 0xee12f906, 0xd277, 0x404b, { 0xb6, 0xda, 0xe5, 0xfa, 0x1a, 0x5f, 0xa0, 0x5b } };

/* Activate High/Ultimate Performance Plan */
bool UnlockMotherboardPowerScheme(void) {
    if (PowerSetActiveScheme(NULL, &GUID_MAX_PERFORMANCE) == ERROR_SUCCESS) {
        printf("[+] Windows Motherboard Power Scheme set to MAXIMUM PERFORMANCE.\n");
        return true;
    }
    printf("[-] Failed to force Maximum Performance power scheme.\n");
    return false;
}

/* Disable PCIe Link State Power Management (ASPM) for minimum NVMe & GPU latency */
bool DisablePCIePowerManagement(void) {
    DWORD aspmlimit = 0; /* 0 = OFF / Disabled */
    
    DWORD status = PowerWriteACValueIndex(
        NULL,
        &GUID_MAX_PERFORMANCE,
        &GUID_PCIEXPRESS_SETTINGS,
        &GUID_ASPM_SETTING,
        aspmlimit
    );

    if (status == ERROR_SUCCESS) {
        PowerApplyPowerRequest(NULL, PowerRequestExecution);
        printf("[+] PCIe ASPM (Active State Power Management) DISABLED across bus lanes.\n");
        return true;
    }

    printf("[-] Failed to modify PCIe power states. Run as Administrator.\n");
    return false;
}

/* Master Windows Motherboard Optimizer */
int main(void) {
    printf("===========================================\n");
    printf("   WINDOWS MOTHERBOARD & CHIPSET MAX OPT   \n");
    printf("===========================================\n");

    UnlockMotherboardPowerScheme();
    DisablePCIePowerManagement();

    printf("===========================================\n");
    printf("[SUCCESS] Motherboard bus latency and power limits removed.\n");
    return 0;
}
