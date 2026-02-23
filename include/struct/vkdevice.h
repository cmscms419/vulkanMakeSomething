#ifndef INCLUDE_STRUCT_VULKAN_TYPE_H_
#define INCLUDE_STRUCT_VULKAN_TYPE_H_

#include "common.h"

struct QueueFamilyIndices {
    cUint32_t graphicsAndComputeFamily = 0;  // 그래픽스/컴퓨팅 큐 패밀리 인덱스 (그래픽스/컴퓨팅 명령을 처리하는 큐)
    cUint32_t presentFamily = 0;             // 프레젠트 큐 패밀리 인덱스 (윈도우 시스템과 Vulkan을 연결하는 인터페이스)
    VkQueueFamilyProperties queueFamilyProperties = {};

    cBool graphicsAndComputeFamilyHasValue = false;
    cBool presentFamilyHasValue = false;

    cVoid setgraphicsAndComputeFamily(cUint32_t index) {
        graphicsAndComputeFamily = index;
        graphicsAndComputeFamilyHasValue = true;
    }
    cVoid setPresentFamily(cUint32_t index) {
        presentFamily = index;
        presentFamilyHasValue = true;
    }
    cUint32_t getGraphicsQueueFamilyIndex() const {
        cUint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsAndComputeFamily;
        }

        return target;
    }
    cUint32_t getPresentQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }
    cUint32_t getComputeQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
        {
            target = graphicsAndComputeFamily;
        }
        return target;
    }
    cBool isComplete() const {
        return this->graphicsAndComputeFamilyHasValue && this->presentFamilyHasValue;
    }
    cVoid reset() {
        this->graphicsAndComputeFamily = 0;
        this->presentFamily = 0;
        this->graphicsAndComputeFamilyHasValue = false;
        this->presentFamilyHasValue = false;
        this->queueFamilyProperties = {};
    }
};

struct QueueFamilyIndices2 {
    cUint32_t graphicFamily = 0;  // 그래픽스 큐 패밀리 인덱스 (그래픽스 명령을 처리하는 큐)
    cUint32_t computerFamily = 0; // 컴퓨팅 큐 패밀리 인덱스 (컴퓨팅 명령을 처리하는 큐)
    cUint32_t transferFamily = 0; // 트랜스퍼 큐 패밀리 인덱스 (트랜스퍼 명령을 처리하는 큐)
    VkQueueFamilyProperties queueFamilyProperties = {};

    cBool graphicFamilyHasValue = false;
    cBool computerFamilyHasValue = false;
    cBool transferFamilyHasValue = false;

    cVoid setGraphicFamily(cUint32_t index) {
        graphicFamily = index;
        graphicFamilyHasValue = true;
    }
    cVoid setComputerFamily(cUint32_t index) {
        computerFamily = index;
        computerFamilyHasValue = true;
    }
    cVoid setTransferFamily(cUint32_t index) {
        transferFamily = index;
        transferFamilyHasValue = true;
    }

    cUint32_t getGraphicQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicFamily;
        }
        return target;
    }
    cUint32_t getComputerQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
        {
            target = computerFamily;
        }
        return target;
    }
    cUint32_t getTransferQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
        {
            target = transferFamily;
        }
        return target;
    }

    cBool isComplete() const {
        return this->graphicFamilyHasValue && this->computerFamilyHasValue && this->transferFamilyHasValue;
    }
    cVoid reset() {
        this->graphicFamily = 0;
        this->computerFamily = 0;
        this->transferFamily = 0;
        this->graphicFamilyHasValue = false;
        this->computerFamilyHasValue = false;
        this->transferFamilyHasValue = false;
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct FrameData {
    FrameData() {
        mainCommandBuffer = VK_NULL_HANDLE;
        VkimageavailableSemaphore = VK_NULL_HANDLE;
        VkrenderFinishedSemaphore = VK_NULL_HANDLE;
        VkinFlightFences = VK_NULL_HANDLE;
    }

    VkCommandBuffer mainCommandBuffer;
    VkSemaphore VkimageavailableSemaphore;
    VkSemaphore VkrenderFinishedSemaphore;
    VkFence VkinFlightFences;

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        info.pInheritanceInfo = nullptr;
        return info;
    }
};

struct depthStencil {
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    VkImage depthImage{ VK_NULL_HANDLE };
    VkDeviceMemory depthImageMemory{ VK_NULL_HANDLE };
    VkImageView depthImageView{ VK_NULL_HANDLE };

    cVoid cleanup(VkDevice device) const {
        
        if (depthImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, depthImageView, nullptr);
        }
        if (depthImage != VK_NULL_HANDLE)
        {
            vkDestroyImage(device, depthImage, nullptr);
        }
        if (depthImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, depthImageMemory, nullptr);
        }
    }
};

struct ComputerFrameData {

    ComputerFrameData() {
        mainCommandBuffer = VK_NULL_HANDLE;
        computeFinishedSemaphores = VK_NULL_HANDLE;
        VkinFlightFences = VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        info.pInheritanceInfo = nullptr;
        return info;
    }

    VkCommandBuffer mainCommandBuffer;
    VkSemaphore computeFinishedSemaphores;
    VkFence VkinFlightFences;
};

struct subUniformBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    cVoid* Mapped;

    cVoid cleanup(VkDevice device) const {
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

#endif // INCLUDE_STRUCT_VULKAN_TYPE_H_