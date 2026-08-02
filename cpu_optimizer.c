#include <windows.h>
#include <processthreadsapi.h>
#include <timeapi.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "winmm.lib")

/* Structure to hold processor topology info */
typedef struct {
    DWORD totalLogicalProcessors;
    DWORD totalPhysicalCores;
    DWORD_PTR systemAffinityMask;
} ProcessorTopology;

/* Function to query and display processor topology */
bool GetProcessorInfo(ProcessorTopology* info) {
    if (!info) return false;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    info->totalLogicalProcessors = sysInfo.dwNumberOfProcessors;
    info->systemAffinityMask = sysInfo.dwActiveProcessorMask;
    info->totalPhysicalCores = 0;

    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(NULL, &bufferSize);

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(bufferSize);
    if (buffer != NULL && GetLogicalProcessorInformation(buffer, &bufferSize)) {
        DWORD count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        for (DWORD i = 0; i < count; i++) {
            if (buffer[i].Relationship == RelationProcessorCore) {
                info->totalPhysicalCores++;
            }
        }
        free(buffer);
    } else {
        if (buffer) free(buffer);
        info->totalPhysicalCores = info->totalLogicalProcessors; /* Fallback */
    }

    return true;
}

/* Elevate process and main thread priority to maximum */
bool OptimizeProcessPriority(void) {
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    /* Set process priority to REALTIME */
    if (!SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS)) {
        /* Fallback to HIGH if REALTIME privileges are denied */
        if (!SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
            printf("[-] Failed to elevate process priority class.\n");
            return false;
        }
        printf("[+] Process priority set to HIGH_PRIORITY_CLASS.\n");
    } else {
        printf("[+] Process priority set to REALTIME_PRIORITY_CLASS.\n");
    }

    /* Set thread priority to TIME_CRITICAL */
    if (!SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL)) {
        printf("[-] Failed to elevate thread priority.\n");
        return false;
    }
    printf("[+] Main thread priority set to THREAD_PRIORITY_TIME_CRITICAL.\n");

    return true;
}

/* Bind process execution across all available logical processors */
bool OptimizeCPUAffinity(const ProcessorTopology* info) {
    HANDLE hProcess = GetCurrentProcess();

    if (!SetProcessAffinityMask(hProcess, info->systemAffinityMask)) {
        printf("[-] Failed to set full processor affinity mask.\n");
        return false;
    }
    printf("[+] Process affinity locked across all %LU logical processors (Mask: 0x%IX).\n",
           info->totalLogicalProcessors, info->systemAffinityMask);

    return true;
}

/* Request highest system timer resolution (1ms) for minimal scheduling latency */
bool OptimizeSystemTimer(void) {
    MMRESULT result = timeBeginPeriod(1);
    if (result == TIMERR_NOERROR) {
        printf("[+] System timer resolution set to maximum precision (1ms).\n");
        return true;
    }
    printf("[-] Failed to adjust system timer resolution.\n");
    return false;
}

/* Disable Power Throttling on Windows 10/11 for maximum execution rate */
bool DisablePowerThrottling(void) {
    HANDLE hProcess = GetCurrentProcess();
    
    PROCESS_POWER_THROTTLING_STATE PowerThrottling;
    RtlZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = 0; /* Clear flags to disable throttling */

    if (SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling))) {
        printf("[+] Power throttling successfully disabled for maximum core frequency.\n");
        return true;
    }
    
    printf("[!] Power throttling adjustment unsupported or requires administrator privileges.\n");
    return false;
}

/* Master function to run all CPU optimization routines */
bool MaximizeProcessorPerformance(void) {
    printf("===========================================\n");
    printf("  INITIALIZING FULL PROCESSOR OPTIMIZATION \n");
    printf("===========================================\n");

    ProcessorTopology topology;
    if (GetProcessorInfo(&topology)) {
        printf("[i] Detected Physical Cores: %LU | Logical Processors: %LU\n",
               topology.totalPhysicalCores, topology.totalLogicalProcessors);
    }

    bool pPriority = OptimizeProcessPriority();
    bool pAffinity = OptimizeCPUAffinity(&topology);
    bool pTimer    = OptimizeSystemTimer();
    bool pPower    = DisablePowerThrottling();

    printf("===========================================\n");
    if (pPriority && pAffinity && pTimer) {
        printf("[SUCCESS] All processor optimization parameters applied successfully.\n");
        return true;
    }

    printf("[WARNING] Optimization completed with partial settings.\n");
    return false;
}

int main(void) {
    MaximizeProcessorPerformance();
    
    /* Program execution context runs at max optimization here */
    
    /* Clean up timer resolution before exiting */
    timeEndPeriod(1);
    return 0;
}
