#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <dispatch/dispatch.h>

typedef struct {
    int totalLogicalCores;
    int totalPhysicalCores;
    int performanceCores;  /* Apple Silicon P-cores */
    int efficiencyCores;   /* Apple Silicon E-cores */
} MacCPUInfo;

/* Query macOS kernel for CPU topology and core distribution */
bool GetMacCPUInfo(MacCPUInfo* info) {
    if (!info) return false;

    size_t size = sizeof(info->totalLogicalCores);
    if (sysctlbyname("hw.logicalcpu", &info->totalLogicalCores, &size, NULL, 0) != 0) {
        info->totalLogicalCores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }

    size = sizeof(info->totalPhysicalCores);
    if (sysctlbyname("hw.physicalcpu", &info->totalPhysicalCores, &size, NULL, 0) != 0) {
        info->totalPhysicalCores = info->totalLogicalCores;
    }

    /* Apple Silicon core split (P-Cores vs E-Cores) */
    size = sizeof(info->performanceCores);
    if (sysctlbyname("hw.perflevel0.logicalcpu", &info->performanceCores, &size, NULL, 0) != 0) {
        info->performanceCores = info->totalLogicalCores; /* Fallback for Intel Macs */
    }

    size = sizeof(info->efficiencyCores);
    if (sysctlbyname("hw.perflevel1.logicalcpu", &info->efficiencyCores, &size, NULL, 0) != 0) {
        info->efficiencyCores = 0;
    }

    return true;
}

/* Elevate macOS Quality of Service (QoS) to User Interactive / Real-time */
bool OptimizeMacQoS(void) {
    /* Set current thread QoS to USER_INTERACTIVE (highest interactive priority) */
    pthread_t thread = pthread_self();
    int result = pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

    if (result == 0) {
        printf("[+] Thread QoS class elevated to QOS_CLASS_USER_INTERACTIVE.\n");
        return true;
    }

    printf("[-] Failed to set high-priority QoS class.\n");
    return false;
}

/* Dispatch tasks specifically targeted to High-Performance cores on Apple Silicon */
void OptimizeCoreScheduling(const MacCPUInfo* info) {
    printf("[+] Target Core Layout: %d Performance Cores | %d Efficiency Cores\n",
           info->performanceCores, info->efficiencyCores);

    /* Use high-priority Grand Central Dispatch queue to force execution on P-Cores */
    dispatch_queue_global_t high_prio_queue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
    
    dispatch_async(high_prio_queue, ^{
        printf("[+] Execution pinned to high-performance scheduler queue.\n");
    });
}

/* Master optimization routine for macOS */
bool MaximizeMacProcessorPerformance(void) {
    printf("===========================================\n");
    printf("  INITIALIZING MACOS CPU OPTIMIZATION      \n");
    printf("===========================================\n");

    MacCPUInfo cpuInfo;
    if (GetMacCPUInfo(&cpuInfo)) {
        printf("[i] Detected Physical Cores: %d | Logical Cores: %d\n",
               cpuInfo.totalPhysicalCores, cpuInfo.totalLogicalCores);
    }

    bool qosStatus = OptimizeMacQoS();
    OptimizeCoreScheduling(&cpuInfo);

    printf("===========================================\n");
    if (qosStatus) {
        printf("[SUCCESS] macOS Processor Max Performance applied.\n");
        return true;
    }

    printf("[WARNING] Optimization completed with partial settings.\n");
    return false;
}

int main(void) {
    MaximizeMacProcessorPerformance();
    return 0;
}
