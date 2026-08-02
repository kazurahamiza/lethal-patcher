#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define BUFFER_SIZE (1024 * 1024 * 64) /* 64 MB Direct VRAM Allocation */

/* Initialize Direct3D 12 Device with Low-Latency Parameters */
ID3D12Device* InitializeD3D12Device(void) {
    IDXGIFactory4* factory = NULL;
    CreateDXGIFactory1(&IID_IDXGIFactory4, (void**)&factory);

    IDXGIAdapter1* adapter = NULL;
    ID3D12Device* device = NULL;

    for (UINT i = 0; factory->lpVtbl->EnumAdapters1(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->lpVtbl->GetDesc1(adapter, &desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue; /* Skip software rasterizer */

        if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, (void**)&device))) {
            wprintf(L"[+] Selected GPU Device: %s\n", desc.Description);
            adapter->lpVtbl->Release(adapter);
            break;
        }
        adapter->lpVtbl->Release(adapter);
    }

    factory->lpVtbl->Release(factory);
    return device;
}

/* Allocate Zero-Copy GPU Upload Heap mapped over Resizable BAR */
bool AllocateZeroCopyGPUBuffer(ID3D12Device* device) {
    if (!device) return false;

    /* Define D3D12 Custom Heap targeting GPU Device-Local + Host-Visible Memory */
    D3D12_HEAP_PROPERTIES heapProps = {0};
    heapProps.Type = D3D12_HEAP_TYPE_GPU_UPLOAD; /* Requires Resizable BAR / Agility SDK */
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc = {0};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = BUFFER_SIZE;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* gpuBuffer = NULL;

    HRESULT hr = device->lpVtbl->CreateCommittedResource(
        device,
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        NULL,
        &IID_ID3D12Resource,
        (void**)&gpuBuffer
    );

    if (SUCCEEDED(hr)) {
        printf("[+] Direct Resizable BAR VRAM Allocation Successful (%d MB mapped).\n", BUFFER_SIZE / (1024 * 1024));

        /* Obtain direct host pointer for zero-copy CPU writes directly to VRAM */
        void* mappedVRAMPointer = NULL;
        D3D12_RANGE readRange = {0, 0}; /* No reading needed from CPU */
        
        if (SUCCEEDED(gpuBuffer->lpVtbl->Map(gpuBuffer, 0, &readRange, &mappedVRAMPointer))) {
            printf("[+] Direct VRAM CPU Write Address Obtained: %p\n", mappedVRAMPointer);
            gpuBuffer->lpVtbl->Unmap(gpuBuffer, 0, NULL);
        }

        gpuBuffer->lpVtbl->Release(gpuBuffer);
        return true;
    }

    printf("[-] Failed to create GPU Upload Heap. Falling back to standard UPLOAD heap.\n");
    return false;
}

int main(void) {
    printf("===========================================\n");
    printf("   WINDOWS D3D12 REBAR / ZERO-COPY ENGINE  \n");
    printf("===========================================\n");

    ID3D12Device* device = InitializeD3D12Device();
    if (device) {
        AllocateZeroCopyGPUBuffer(device);
        device->lpVtbl->Release(device);
    }

    printf("===========================================\n");
    return 0;
}
