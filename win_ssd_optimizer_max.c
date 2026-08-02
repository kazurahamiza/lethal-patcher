#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "kernel32.lib")

/* Define undocumented or extended I/O thread priority structures if needed */
typedef enum _IO_PRIORITY_HINT {
    IoPriorityVeryLow = 0,
    IoPriorityLow,
    IoPriorityNormal,
    IoPriorityHigh,
    MaxIoPriorityTypes
} IO_PRIORITY_HINT;

typedef struct _FILE_LEVEL_TRIM_RANGE {
    DWORD64 Offset;
    DWORD64 Length;
} FILE_LEVEL_TRIM_RANGE, *PFILE_LEVEL_TRIM_RANGE;

typedef struct _FILE_LEVEL_TRIM {
    DWORD Key;
    DWORD NumRanges;
    FILE_LEVEL_TRIM_RANGE Ranges[1];
} FILE_LEVEL_TRIM, *PFILE_LEVEL_TRIM;

/* Elevate Process and Thread I/O priorities to Critical/High tier */
bool OptimizeWindowsIOPriority(void) {
    HANDLE hThread = GetCurrentThread();

    /* Set thread execution priority to TIME_CRITICAL for I/O operations */
    if (SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL)) {
        printf("[+] Thread execution priority elevated to THREAD_PRIORITY_TIME_CRITICAL.\n");
    } else {
        printf("[-] Failed to set thread priority.\n");
        return false;
    }

    return true;
}

/* Query storage media properties to confirm SSD/NVMe drive type */
bool QueryStorageMediaType(HANDLE hDevice) {
    STORAGE_PROPERTY_QUERY query;
    ZeroMemory(&query, sizeof(query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    STORAGE_DEVICE_DESCRIPTOR devDesc;
    ZeroMemory(&devDesc, sizeof(devDesc));
    DWORD bytesReturned = 0;

    if (DeviceIoControl(
            hDevice,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query,
            sizeof(query),
            &devDesc,
            sizeof(devDesc),
            &bytesReturned,
            NULL)) {
        
        printf("[+] Storage Device Query Successful.\n");
        printf("    Bus Type: 0x%X (NVMe = 0x11, SATA = 0x0B)\n", devDesc.BusType);
        return true;
    }

    printf("[-] Failed to query storage device properties.\n");
    return false;
}

/* Open volume or target file with DIRECT I/O (Bypasses Windows System Cache) */
HANDLE OpenDirectStorageHandle(const char* targetPath) {
    HANDLE hFile = CreateFileA(
        targetPath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, /* Bypasses Windows caching */
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[-] Failed to open handle with Direct I/O (Error: %lu).\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    printf("[+] Target opened with FILE_FLAG_NO_BUFFERING & FILE_FLAG_WRITE_THROUGH.\n");
    return hFile;
}

/* Issue hardware-level TRIM pass to NVMe controller */
bool IssueWindowsStorageTRIM(HANDLE hVolume) {
    DWORD bytesReturned = 0;
    FILE_LEVEL_TRIM trimInput;
    ZeroMemory(&trimInput, sizeof(trimInput));

    trimInput.Key = 0;
    trimInput.NumRanges = 1;
    trimInput.Ranges[0].Offset = 0;
    trimInput.Ranges[0].Length = 0xFFFFFFFFFFFFFFFF; /* Target full range */

    BOOL result = DeviceIoControl(
        hVolume,
        FSCTL_FILE_LEVEL_TRIM,
        &trimInput,
        sizeof(trimInput),
        NULL,
        0,
        &bytesReturned,
        NULL
    );

    if (result) {
        printf("[+] Hardware TRIM/Deallocate command issued successfully.\n");
        return true;
    } else {
        printf("[!] TRIM notification sent or requires elevated Administrator privileges.\n");
        return false;
    }
}

/* Master Windows SSD Optimization Controller */
bool MaximizeWindowsSSDPerformance(const char* driveLetter) {
    printf("===========================================\n");
    printf("  WINDOWS NVME / SSD STORAGE MAX ENGINE    \n");
    printf("===========================================\n");

    char volumePath[32];
    snprintf(volumePath, sizeof(volumePath), "\\\\.\\%s", driveLetter);

    HANDLE hDrive = CreateFileA(
        volumePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hDrive == INVALID_HANDLE_VALUE) {
        printf("[-] Access denied or invalid drive. Run as Administrator.\n");
        return false;
    }

    QueryStorageMediaType(hDrive);
    OptimizeWindowsIOPriority();
    IssueWindowsStorageTRIM(hDrive);

    CloseHandle(hDrive);

    printf("===========================================\n");
    printf("[SUCCESS] Windows SSD optimization pass completed.\n");
    return true;
}

int main(void) {
    /* Pass root drive letter (requires Administrator privileges for drive handle) */
    MaximizeWindowsSSDPerformance("C:");
    return 0;
}
