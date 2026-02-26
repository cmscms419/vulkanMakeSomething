#ifndef INCLUDE_HELPER_DEVICE_H_
#define INCLUDE_HELPER_DEVICE_H_

#include "common.h"
#include "vkdevice.h"

#include <set>

namespace vkengine
{
    namespace helper
    {
        namespace device
        {
            // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수 (surface 포함) -> swapchain 지원 여부 포함
            cBool isDeviceSuitableWithSurface(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices &indices);

            // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수
            cBool isDeviceSuitableWithoutSurface(VkPhysicalDevice device, QueueFamilyIndices2 &indices);

            // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
            // 검증 레이어 지원 여부를 확인하는 함수
            // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
            cBool checkDeviceExtensionSupport(VkPhysicalDevice device);

            // 물리 디바이스의 모든 확장기능을 가져오는 함수
            void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<std::string> *temp);

            cString getPhysicalDeviceTypeString(VkPhysicalDeviceType type);

            // 물리 디바이스의 적합성을 평가하는 함수
            int rateDeviceSuitability(VkPhysicalDevice device);

            // 스왑 체인 지원 정보를 가져오는 함수
            // 스왑 체인 지원 정보를 저장할 구조체를 초기화
            // 물리 장치에서 서피스의 기능을 가져옴
            const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

            // 깊이 형식을 찾는 함수
            VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

            // Format을 지원하는지 확인하는 함수
            VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

            cUint32_t findMemoryType(VkPhysicalDevice physicalDevice, cUint32_t typeFilter, VkMemoryPropertyFlags properties);

            // 최대 사용 가능한 샘플링 수를 반환하는 함수
            VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

            // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
            // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
            // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
            const QueueFamilyIndices findQueueFamiliesWithSurface(VkPhysicalDevice &device, VkSurfaceKHR &VKsurface);

            const QueueFamilyIndices2 findQueueFamiliesWithoutSurface(VkPhysicalDevice &device);

            // pyhsicalDeviceProperties를 가져오는 함수
            void getPyhsicalDeviceProperties(VkPhysicalDevice device);

            // Fence에 대한 information을 생성하는 함수
            VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

            // Semaphore에 대한 information을 생성하는 함수
            VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

            // commandPood에 대한 information을 생성하는 함수
            VkCommandPoolCreateInfo commandPoolCreateInfo(cUint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

            // commandBuffer에 대한 information을 생성하는 함수
            VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, cUint32_t count, VkCommandBufferLevel level);
        }
    }
}

#endif // !INCLUDE_HELPER_DEVICE_H_