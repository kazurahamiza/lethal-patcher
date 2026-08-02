#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ALLOCATION_SIZE (1024 * 1024 * 64) /* 64 MB */

/* Locate Memory Type supporting Host-Visible Device Local VRAM (ReBAR) */
int FindHostVisibleDeviceLocalMemoryType(VkPhysicalDeviceMemoryProperties* memProps, uint32_t typeBits) {
    VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    for (uint32_t i = 0; i < memProps->memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && ((memProps->memoryTypes[i].propertyFlags & requiredFlags) == requiredFlags)) {
            return i;
        }
    }

    /* Fallback: Host-Visible only */
    for (uint32_t i = 0; i < memProps->memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (memProps->memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            return i;
        }
    }

    return -1;
}

bool AllocateVulkanZeroCopyVRAM(VkDevice device, VkPhysicalDevice physicalDevice) {
    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = ALLOCATION_SIZE;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, NULL, &buffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    int memoryTypeIndex = FindHostVisibleDeviceLocalMemoryType(&memProps, memReqs.memoryTypeBits);
    if (memoryTypeIndex < 0) {
        printf("[-] Target Memory Type not available.\n");
        vkDestroyBuffer(device, buffer, NULL);
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = (uint32_t)memoryTypeIndex;

    VkDeviceMemory bufferMemory;
    if (vkAllocateMemory(device, &allocInfo, NULL, &bufferMemory) == VK_SUCCESS) {
        vkBindBufferMemory(device, buffer, bufferMemory, 0);

        void* mappedPtr = NULL;
        vkMapMemory(device, bufferMemory, 0, ALLOCATION_SIZE, 0, &mappedPtr);
        printf("[+] Vulkan Resizable BAR Memory Mapped at Host Address: %p\n", mappedPtr);

        vkUnmapMemory(device, bufferMemory);
        vkFreeMemory(device, bufferMemory, NULL);
        vkDestroyBuffer(device, buffer, NULL);
        return true;
    }

    vkDestroyBuffer(device, buffer, NULL);
    return false;
}

int main(void) {
    printf("===========================================\n");
    printf("   VULKAN ZERO-COPY / REBAR VRAM ENGINE    \n");
    printf("===========================================\n");
    printf("[i] Module ready for integration into Vulkan rendering runtime.\n");
    printf("===========================================\n");
    return 0;
}
