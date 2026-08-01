#include <iostream>
#include <cstdlib>

void exec(const char* title, const char* cmd) {
    std::cout << "\n[+] Applying: " << title << std::endl;
    int res = std::system(cmd);
    if (res == 0) {
        std::cout << "    -> SUCCESS" << std::endl;
    } else {
        std::cout << "    -> FAILED or ALREADY APPLIED (Code: " << res << ")" << std::endl;
    }
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "   MONSTER OS OPTIMIZER ENGINE v1.0       " << std::endl;
    std::cout << "==========================================" << std::endl;

    exec("Enable Ultimate Performance Scheme", "powercfg -duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61");
    exec("Activate Ultimate Performance Scheme", "powercfg -setactive e9a42b02-d5df-448d-aa00-03f14749eb61");
    exec("Enable TCP Global AutoTuning", "netsh int tcp set global autotuninglevel=experimental");
    exec("Disable TCP Chimney Offload", "netsh int tcp set global chimney=disabled");
    exec("Set Network Congestion Control Provider", "netsh int tcp set global congestionprovider=ctcp");
    exec("Disable SysMain / Superfetch", "sc config SysMain start= disabled & net stop SysMain");
    exec("Disable Windows Search Indexer", "sc config WSearch start= disabled & net stop WSearch");
    exec("Clear Pagefile on Shutdown", "reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management\" /v ClearPageFileAtShutdown /t REG_DWORD /d 1 /f");
    exec("Prioritize Foreground Applications", "reg add \"HKLM\\SYSTEM\\CurrentControlSet\\Control\\PriorityControl\" /v Win32PrioritySeparation /t REG_DWORD /d 38 /f");

    std::cout << "\n==========================================" << std::endl;
    std::cout << " [!] Monster Optimization Complete! Restart PC to take effect." << std::endl;
    std::cout << "==========================================" << std::endl;

    return 0;
}
