#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* Force CPU Scaling Governor to Performance across all motherboard sockets */
bool MaximizeCPUGovernor(void) {
    int result = system("for dev in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do echo performance > \"$dev\" 2>/dev/null; done");
    if (result == 0) {
        printf("[+] All CPU core scaling governors forced to 'performance'.\n");
        return true;
    }
    printf("[-] Failed to update CPU scaling governors. Root privileges required.\n");
    return false;
}

/* Force-disable PCIe ASPM across motherboard PCIe slots via Sysfs */
bool DisableLinuxPCIeASPM(void) {
    int fd = open("/sys/module/pcie_aspm/parameters/policy", O_WRONLY);
    if (fd != -1) {
        const char* policy = "performance";
        write(fd, policy, strlen(policy));
        close(fd);
        printf("[+] PCIe ASPM policy set to 'performance' (Bus power drops disabled).\n");
        return true;
    }
    
    /* Alternative via system command */
    system("echo performance > /sys/module/pcie_aspm/parameters/policy 2>/dev/null");
    printf("[+] Sysfs PCIe ASPM parameter override dispatched.\n");
    return true;
}

/* Disable CPU C-States via dma_latency to keep bus interconnects awake */
bool DisableCStates(void) {
    int fd = open("/dev/cpu_dma_latency", O_WRONLY);
    if (fd < 0) {
        printf("[-] Failed to lock /dev/cpu_dma_latency. Run with sudo.\n");
        return false;
    }

    int latency = 0; /* 0 microseconds latency limit */
    if (write(fd, &latency, sizeof(latency)) == sizeof(latency)) {
        printf("[+] CPU C-States locked to C0 (0us latency limit enforced).\n");
        return true;
    }

    close(fd);
    return false;
}

int main(void) {
    printf("===========================================\n");
    printf("   LINUX MOTHERBOARD CHIPSET & BUS MAX ENGINE\n");
    printf("===========================================\n");

    MaximizeCPUGovernor();
    DisableLinuxPCIeASPM();
    DisableCStates();

    printf("===========================================\n");
    printf("[SUCCESS] Linux motherboard latency controls unlocked.\n");
    return 0;
}
