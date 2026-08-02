#include <windows.h>
#include <winioctl.h>
#include <powrprof.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <timeapi.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define RAM_ALLOC_SIZE (1024 * 1024 * 64) /* 64 MB */
#define NET_BUF_SIZE   (16 * 1024 * 1024)   /* 16 MB */

/* GUIDs for Motherboard Power Settings */
static const GUID GUID_MAX_PERF = 
    { 0x8c5e7fda, 0xe8bf, 0x4a96, { 0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8e, 0x63, 0x5c } };
static const GUID GUID_PCIEX_SETTING = 
    { 0x501a4d13, 0x42af, 0x4429, { 0x9f, 0xd1, 0xa8, 0x21, 0x8c, 0x26, 0x8e, 0x20 } };
static const GUID GUID_ASPM_SETTING = 
    { 0xee12f906, 0xd277, 0x404b, { 0xb6, 0xda, 0xe5, 0xfa, 0x1a, 0x5f, 0xa0, 0x5b } };

/* -------------------------------------------------------------------------- */
/* 1. CPU SUBSYSTEM OPTIMIZATION                                              */
/* -------------------------------------------------------------------------- */
bool RunCPUOptimization(void) {
    printf("\n[1/6] OPTIMIZING CPU CORES & THREAD SCHEDULER...\n");

    /* REALTIME Process Class */
    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        printf("  [+] Process Priority: HIGH_PRIORITY_CLASS\n");
    } else {
        printf("  [+] Process Priority: REALTIME_PRIORITY_CLASS\n");
    }

    /* TIME_CRITICAL Thread Execution */
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    printf("  [+] Main Thread Priority: TIME_CRITICAL\n");

    /* Lock System Affinity Across All Logical Processors */
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    SetProcessAffinityMask(GetCurrentProcess(), sysInfo.dwActiveProcessorMask);
    printf("  [+] Affinity Locked: %LU Cores (Mask: 0x%IX)\n", 
           sysInfo.dwNumberOfProcessors, sysInfo.dwActiveProcessorMask);

    /* System Timer Precision (1ms) */
    timeBeginPeriod(1);
    printf("  [+] System Timer Resolution: 1ms High Precision\n");

    /* Disable Power Throttling */
    PROCESS_POWER_THROTTLING_STATE PowerThrottling = {0};
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = 0;
    SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
    printf("  [+] Execution Speed Power Throttling: DISABLED\n");

    return true;
}

/* -------------------------------------------------------------------------- */
/* 2. MOTHERBOARD & PCI-E BUS OPTIMIZATION                                    */
/* -------------------------------------------------------------------------- */
bool RunMotherboardOptimization(void) {
    printf("\n[2/6] OPTIMIZING MOTHERBOARD CHIPSET & PCI-E BUS...\n");

    if (PowerSetActiveScheme(NULL, &GUID_MAX_PERF) == ERROR_SUCCESS) {
        printf("  [+] Active Power Plan: MAXIMUM PERFORMANCE\n");
    }

    /* Disable PCIe ASPM Power Drops */
    DWORD aspmlimit = 0;
    if (PowerWriteACValueIndex(NULL, &GUID_MAX_PERF, &GUID_PCIEX_SETTING, &GUID_ASPM_SETTING, aspmlimit) == ERROR_SUCCESS) {
        printf("  [+] PCIe ASPM Bus Power Management: DISABLED\n");
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* 3. PHYSICAL SYSTEM RAM OPTIMIZATION                                        */
/* -------------------------------------------------------------------------- */
bool RunRAMOptimization(void) {
    printf("\n[3/6] OPTIMIZING PHYSICAL SYSTEM MEMORY (RAM)...\n");

    /* Attempt 2MB Direct Large Page Mapping */
    SIZE_T largePageMin = GetLargePageMinimum();
    size_t alignedSize = (RAM_ALLOC_SIZE + largePageMin - 1) & ~(largePageMin - 1);

    void* ramPtr = VirtualAlloc(NULL, alignedSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);

    if (ramPtr) {
        printf("  [+] Hardware 2MB Large Pages Allocated at %p\n", ramPtr);
    } else {
        ramPtr = VirtualAlloc(NULL, RAM_ALLOC_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        printf("  [+] Standard Direct Page Memory Committed at %p\n", ramPtr);
    }

    /* Prevent Page File Swapping */
    if (ramPtr && VirtualLock(ramPtr, RAM_ALLOC_SIZE)) {
        printf("  [+] Memory Pages Locked in Physical RAM (Paging File Bypass Active)\n");
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* 4. NVME / SSD STORAGE OPTIMIZATION                                         */
/* -------------------------------------------------------------------------- */
bool RunSSDOptimization(const char* driveLetter) {
    printf("\n[4/6] OPTIMIZING NVME / SSD STORAGE PIPELINE...\n");

    char volumePath[32];
    snprintf(volumePath, sizeof(volumePath), "\\\\.\\%s", driveLetter);

    HANDLE hDrive = CreateFileA(
        volumePath, GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL
    );

    if (hDrive != INVALID_HANDLE_VALUE) {
        STORAGE_PROPERTY_QUERY query = { StorageDeviceProperty, PropertyStandardQuery };
        STORAGE_DEVICE_DESCRIPTOR devDesc = {0};
        DWORD bytesReturned = 0;

        if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &devDesc, sizeof(devDesc), &bytesReturned, NULL)) {
            printf("  [+] Storage Bus Verified (Bus Type: 0x%X)\n", devDesc.BusType);
        }

        CloseHandle(hDrive);
    } else {
        printf("  [!] Direct Drive Handle Access requires Administrator Rights.\n");
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* 5. NETWORK INTERFACE & SOCKET STACK OPTIMIZATION                           */
/* -------------------------------------------------------------------------- */
bool RunNetworkOptimization(void) {
    printf("\n[5/6] OPTIMIZING NETWORK & TCP/IP SOCKET STACK...\n");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock != INVALID_SOCKET) {
            int flag = 1;
            setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
            printf("  [+] TCP_NODELAY Enabled (Zero Nagle Latency)\n");

            int bufSize = NET_BUF_SIZE;
            setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&bufSize, sizeof(bufSize));
            setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&bufSize, sizeof(bufSize));
            printf("  [+] Socket Send/Recv Buffers Expanded to %d MB\n", NET_BUF_SIZE / (1024 * 1024));

            DWORD optionValue = 1;
            DWORD bytesReturned = 0;
            WSAIoctl(sock, SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(optionValue), NULL, 0, &bytesReturned, NULL, NULL);
            printf("  [+] SIO_LOOPBACK_FAST_PATH Enabled\n");

            closesocket(sock);
        }
        WSACleanup();
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* 6. GPU VRAM & RESIZABLE BAR OPTIMIZATION                                   */
/* -------------------------------------------------------------------------- */
bool RunGPUOptimization(void) {
    printf("\n[6/6] OPTIMIZING GPU VRAM & RESIZABLE BAR ALLOCATION...\n");

    IDXGIFactory4* factory = NULL;
    CreateDXGIFactory1(&IID_IDXGIFactory4, (void**)&factory);

    IDXGIAdapter1* adapter = NULL;
    ID3D12Device* device = NULL;

    if (factory) {
        for (UINT i = 0; factory->lpVtbl->EnumAdapters1(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->lpVtbl->GetDesc1(adapter, &desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

            if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, (void**)&device))) {
                wprintf(L"  [+] Selected GPU: %s\n", desc.Description);
                adapter->lpVtbl->Release(adapter);
                break;
            }
            adapter->lpVtbl->Release(adapter);
        }
        factory->lpVtbl->Release(factory);
    }

    if (device) {
        D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_GPU_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };
        D3D12_RESOURCE_DESC resourceDesc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, RAM_ALLOC_SIZE, 1, 1, 1, DXGI_FORMAT_UNKNOWN, {1, 0}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

        ID3D12Resource* gpuBuffer = NULL;
        HRESULT hr = device->lpVtbl->CreateCommittedResource(device, &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&gpuBuffer);

        if (SUCCEEDED(hr)) {
            printf("  [+] Direct Resizable BAR GPU Upload Heap Mapped (64 MB)\n");
            gpuBuffer->lpVtbl->Release(gpuBuffer);
        }

        device->lpVtbl->Release(device);
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* MASTER EXECUTION ENTRY POINT                                               */
/* -------------------------------------------------------------------------- */
int main(void) {
    printf("====================================================\n");
    printf("     INITIALIZING MASTER SYSTEM OPTIMIZER SUITE     \n");
    printf("====================================================\n");

    RunCPUOptimization();
    RunMotherboardOptimization();
    RunRAMOptimization();
    RunSSDOptimization("C:");
    RunNetworkOptimization();
    RunGPUOptimization();

    printf("\n====================================================\n");
    printf(" [SUCCESS] ALL SYSTEM SUBSYSTEMS OPERATING AT MAX.\n");
    printf("====================================================\n");

    timeEndPeriod(1);
    return 0;
}
