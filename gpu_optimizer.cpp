void GPUOptimizer::ApplyNVidiaOptimizations() {
    std::cout << "[+] Injecting Ultra High-Performance NVIDIA Registry & Driver Tweaks..." << std::endl;
    
    HKEY hKey;
    const char* nvPath = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, nvPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        
        // 1. Force PowerMizer to Maximum Performance (Disable Clock Throttling)
        DWORD perfMode = 1;
        RegSetValueExA(hKey, "PowerMizerEnable", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevel", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        RegSetValueExA(hKey, "PowerMizerLevelAC", 0, REG_DWORD, (BYTE*)&perfMode, sizeof(perfMode));
        
        // 2. Disable CUDA P2 State (Allow Full Memory Overclock under Compute/3D loads)
        DWORD disableP2 = 0;
        RegSetValueExA(hKey, "DisableDynamicPState", 0, REG_DWORD, (BYTE*)&disableP2, sizeof(disableP2));

        // 3. Force Ultra Low Latency / Pre-Rendered Frames to Minimum (1 Frame)
        DWORD maxPreRenderedFrames = 1;
        RegSetValueExA(hKey, "MaxPreRenderedFrames", 0, REG_DWORD, (BYTE*)&maxPreRenderedFrames, sizeof(maxPreRenderedFrames));

        // 4. Force High Performance Texture Filtering Quality
        DWORD qualityFilter = 2; // 2 = High Performance Mode
        RegSetValueExA(hKey, "TextureFilteringQuality", 0, REG_DWORD, (BYTE*)&qualityFilter, sizeof(qualityFilter));

        RegCloseKey(hKey);
        std::cout << " [SUCCESS] Applied PowerMizer, Low Latency, and Unlocked Memory P-States." << std::endl;
    } else {
        std::cout << " [WARNING] Failed to write NVIDIA driver registry settings. Run as Administrator." << std::endl;
    }
}
