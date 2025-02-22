#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include "../_common.h"

namespace vkengine {
    namespace helper {

        // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수
        bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices* indices);

        // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
        // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
        // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
        // 검증 레이어 지원 여부를 확인하는 함수
        // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // 물리 디바이스의 모든 확장기능을 가져오는 함수
        void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<std::string>* temp);

        // 물리 디바이스의 적합성을 평가하는 함수
        int rateDeviceSuitability(VkPhysicalDevice device);

        // 스왑 체인 지원 정보를 가져오는 함수
        // 스왑 체인 지원 정보를 저장할 구조체를 초기화
        // 물리 장치에서 서피스의 기능을 가져옴
        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

        // 깊이 형식을 찾는 함수
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

        // Format을 지원하는지 확인하는 함수
        VkFormat findSupportedFormat(
            VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features);

        uint32_t findMemoryType(
            VkPhysicalDevice physicalDevice,
            uint32_t typeFilter,
            VkMemoryPropertyFlags properties);

        VkImageView createImageView(
            VkDevice& device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels);
        
        // 스텐실 컴포넌트를 가지고 있는지 확인하는 함수
        bool hasStencilComponent(VkFormat format);
        
        // Mipmaps을 생성하는 함수
        void generateMipmaps(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels);

        // 최대 사용 가능한 샘플링 수를 반환하는 함수
        
        VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
        
        // setupCommandBuffer 나중에 추가
        // flushSetupCommands 나중에 추가

        // pyhsicalDeviceProperties를 가져오는 함수
        void getPyhsicalDeviceProperties(VkPhysicalDevice device);

        // commandPood에 대한 information을 생성하는 함수
        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

        // commandBuffer에 대한 information을 생성하는 함수
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Fencee에 대한 information을 생성하는 함수
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

        // Semaphore에 대한 information을 생성하는 함수
        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);


        // https://steel-gourd-618.notion.site/Images-19618a41dc6f80b89bc1d1575bcf3d04 참고
        //  시작하려는 명령버퍼를 생성하는 함수
        VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);

        // 명령버퍼를 종료하는 함수
        void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkCommandBuffer& commandBuffer);

        // 이미지 레이아웃을 전환하는 함수
        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels);

        // 이미지 레이아웃을 전환하는 함수(
        //void 

        // stencilComponent를 가지고 있는지 확인하는 함수
        bool hasStencilComponent(VkFormat format);

    }
}

#endif // !INCLUDE_SOURCE_HELPER_H_
