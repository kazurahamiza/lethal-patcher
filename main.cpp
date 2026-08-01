#include <iostream>
#include <cstdlib>
#include "gpu_optimizer.hpp"

void runCommand(const char* label, const char* command) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << " [+] Executing: " << label << std::endl;
    std::cout << "==================================================" << std::endl;

    int result = std::system(command);

    if (result == 0) {
        std::cout << " [SUCCESS] " << label << " completed successfully." << std::endl;
    } else {
        std::cout << " [WARNING] " << label << " exited with code: " << result << std::endl;
    }
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "       LETHAL PATCHER ENGINE v1.0                 " << std::endl;
    std::cout << "   Windows System Repair & GPU Optimization       " << std::endl;
    std::cout << "==================================================" << std::endl;

    // 1. Repair Corrupted System Files
    runCommand("System File Checker (SFC)", "sfc /scannow");

    // 2. Repair Windows Component Store via DISM
    runCommand("DISM Cleanup Image Check", "dism /online /cleanup-image /checkhealth");
    runCommand("DISM Restore Health", "dism /online /cleanup-image /restorehealth");

    // 3. Flush DNS Cache to resolve network glitches
    runCommand("Flush DNS Cache", "ipconfig /flushdns");

    // 4. Execute GPU Overclock & Optimization Engine
    std::cout << "\n==================================================" << std::endl;
    std::cout << " [+] Executing: GPU Overclock & Performance Engine" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    LethalPatcher::GPUOptimizer gpuOpt;
    if (gpuOpt.Initialize()) {
        gpuOpt.OptimizeForMaximumPerformance();
    } else {
        std::cout << " [WARNING] GPU Optimization Engine failed to initialize." << std::endl;
    }

    std::cout << "\n[+] Lethal Patcher execution finished." << std::endl;
    return 0;
}
