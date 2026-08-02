#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Bare-metal command executor function in C
static void exec(const char* title, const char* cmd) {
    printf(" [+] Applying: %s\n", title);
    int res = system(cmd);
    if (res == 0) {
        printf("     -> SUCCESS\n");
    } else {
        printf("     -> FAILED or ALREADY APPLIED (Code: %d)\n", res);
    }
}

int main(void) {
    printf("==================================================\n");
    printf("       MONSTER OS OPTIMIZER ENGINE v1.0 (RAW C)   \n");
    printf("==================================================\n");

    exec("Enable Ultimate Performance Scheme", "powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 >nul 2>&1");
    exec("Activate Ultimate Performance Scheme", "powercfg -setactive e9a42b02-d5df-448d-aa00-03f14749eb61 >nul 2>&1");
    exec("Enable TCP Global AutoTuning", "netsh int tcp set global autotuninglevel=experimental >nul 2>&1");
    exec("Disable TCP Chimney Offload", "netsh int tcp set global chimney=disabled >nul 2>&1");
    exec("Set Network Congestion Control Provider", "netsh int tcp set global congestionprovider=ctcp >nul 2>&1");
    exec("Disable SysMain / Superfetch", "sc config SysMain start= disabled >nul 2>&1 && net stop SysMain >nul 2>&1");
    exec("Disable Windows Search Indexer", "sc config WSearch start= disabled >nul 2>&1 && net stop WSearch >nul 2>&1");
    exec("Clear Pagefile on Shutdown", "reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management\" /v ClearPageFileAtShutdown /t REG_DWORD /d 1 /f >nul 2>&1");
    exec("Prioritize Foreground Applications", "reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\PriorityControl\" /v Win32PrioritySeparation /t REG_DWORD /d 38 /f >nul 2>&1");

    printf("\n==================================================\n");
    printf(" [!] Monster Optimization Complete! Restart PC to take effect.\n");
    printf("==================================================\n");

    return 0;
}
