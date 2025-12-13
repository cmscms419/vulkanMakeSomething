#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include <set>
#include <fstream>

#include "common.h"
#include "struct.h"
#include "macros.h"
#include "log.h"

#include "ktx.h"

#include "helperCreate.h"
#include "helperFile.h"


// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine {

    namespace helper {

        cString shaderStageFlagsToString(VkShaderStageFlags flags);

        cUint32_t getFormatSize(VkFormat format);

        VkFormat getVkFormatFromSpvReflectFormat(SpvReflectFormat format);

        void printReflectionInfo(const SpvReflectShaderModule& reflectModule);

        cString descriptorTypeToString(VkDescriptorType type);

        VkDescriptorType stringToDescriptorType(const cString& typeStr);

        const cChar* getShaderStageString(const SpvReflectShaderStageFlagBits& stage);

        const cChar* getDescriptorTypeString(SpvReflectDescriptorType type);

        cString getPhysicalDeviceTypeString(VkPhysicalDeviceType type);

        VkAccessFlags getFromOldLayoutToVkAccessFlags(VkImageLayout format);

        VkAccessFlags getFromNewLayoutToVkAccessFlags(VkImageLayout format);

        // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수 (surface 포함) -> swapchain 지원 여부 포함
        cBool isDeviceSuitableWithSurface(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices);

        // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수 
        cBool isDeviceSuitableWithoutSurface(VkPhysicalDevice device, QueueFamilyIndices2& indices);

        // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
        // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
        // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
        const QueueFamilyIndices findQueueFamiliesWitchSurface(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface);

        const QueueFamilyIndices2 findQueueFamiliesWithoutSurface(VkPhysicalDevice& device);

        // 버퍼를 복사하는 함수
        void copyBuffer(
            VkDevice VKdevice,
            VkCommandPool VKcommandPool,
            VkQueue graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        // buffer를 생성하는 함수
        void createBuffer(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        void copyBufferToImage(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            cUint32_t width,
            cUint32_t height);

        void copyBufferToImage2(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            cUint32_t width,
            cUint32_t height,
            std::vector<VkDeviceSize> &sizeArray);

        void copyBufferToImage3(
            VkCommandBuffer cmb,
            VkBuffer buffer,
            VkImage image,
            cUint32_t width,
            cUint32_t height);

        void copyBufferToImageKTX(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            cUint32_t width,
            cUint32_t height,
            cUint32_t mipmapLevels,
            ktxTexture* textureKTX);

        void copyBufferToImageKTX2(
            VkCommandBuffer cmd,
            VkBuffer buffer,
            VkImage image,
            cUint32_t width,
            cUint32_t height,
            cUint32_t mipmapLevels,
            ktxTexture* textureKTX,
            cBool useCubmap = false);

        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
        // 검증 레이어 지원 여부를 확인하는 함수
        // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
        cBool checkDeviceExtensionSupport(VkPhysicalDevice device);

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

        cUint32_t findMemoryType(
            VkPhysicalDevice physicalDevice,
            cUint32_t typeFilter,
            VkMemoryPropertyFlags properties);

        // 스텐실 컴포넌트를 가지고 있는지 확인하는 함수
        inline cBool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        // Mipmaps을 생성하는 함수
        // 이미지를 생성하고 이미지 레이아웃을 변경한 다음 이미지를 복사 -> 단일 이미지에 대한 mipmap 생성
        void generateMipmaps(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            cUint32_t mipLevels);

        void getGenerateMipmapsInfo(
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            cUint32_t mipLevels
        );

        // CubeMap에 대한 mipmap을 생성하는 함수
        void generateMipmapsCubeMap(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            cUint32_t mipLevels);

        void generateMipmapsCubeMap2(
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            cUint32_t mipLevels
        );



        // 최대 사용 가능한 샘플링 수를 반환하는 함수

        //VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer 나중에 추가
        // flushSetupCommands 나중에 추가

        // pyhsicalDeviceProperties를 가져오는 함수
        inline void getPyhsicalDeviceProperties(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            _PRINT_TO_CONSOLE_("Device Name: %s\n", deviceProperties.deviceName);
            _PRINT_TO_CONSOLE_("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        }

        // https://steel-gourd-618.notion.site/Images-19618a41dc6f80b89bc1d1575bcf3d04 참고
        // 시작하려는 명령버퍼를 생성하는 함수
        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

        // 명령버퍼를 종료하는 함수
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue Queue, VkCommandBuffer commandBuffer);
        
        /**
            * @brief 이미지의 레이아웃을 전환하는 함수
            *
            * 이 함수는 지정된 이미지에 대해 필요에 따른 이미지 메모리 배리어를 설정하고,
            * 기존 레이아웃(oldLayout)에서 새로운 레이아웃(newLayout)으로 이미지를 변경합니다.
            * 이미지 전환은 단일 시간 명령 버퍼를 사용하여 수행되며, 전환 작업 후 관련 명령 버퍼는
            * 제출되어 완료될 때까지 대기됩니다.
            *
            * ->이전 oldLayout을 가진 imageLayout을 newLayout으로 이미지를 사용 할 때 이용하는 함수
            *
            * @remark 지원되는 전환은 아래의 3가지 경우입니다:
            *  1) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            *  2) VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            *  3) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            *  4) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            *
         */
        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            cUint32_t levelCount = 1,
            cUint32_t layerCount = 1
        );

        /**
            * @brief 이미지 레이아웃 전환 명령을 커맨드 버퍼에 기록하는 함수
            *
            * 이 함수는 주어진 VkCommandBuffer에 이미지 레이아웃 전환을 위한 VkImageMemoryBarrier를 기록합니다.
            * oldLayout에서 newLayout으로의 전환을 위해 적절한 접근 마스크와 파이프라인 스테이지를 설정합니다.
            * mipmap level과 array layer 범위도 지정할 수 있습니다.
            *
        */
        void updateimageLayoutcmd(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            cUint32_t levelCount = 1,
            cUint32_t layerCount = 1
        );

        // stencilComponent를 가지고 있는지 확인하는 함수
        cBool hasStencilComponent(VkFormat format);

        // 특정 데이터를 VkDeviceMemory 객체로 복사하는 함수
        template <typename T>
        inline void copyToDeviceMemory(VkDevice device, const T* src, VkDeviceMemory dst, VkDeviceSize size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0)
        {
            void* data;
            vkMapMemory(device, dst, offset, size, flags, &data);
            memcpy(data, src, (size_t)size);
            vkUnmapMemory(device, dst);
        }

        cUint32_t alignedSize(cUint32_t value, cUint32_t alignment);
        
        size_t alignedSize(size_t value, size_t alignment);
        
        VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment);

        // 그래픽 파이프라인 생성

        // 동기화 객체 초기화
        void initializeSynchronization(VkDevice device, cUint32_t maxFramesInFlight, cUint32_t imageCount,
            std::vector<VkSemaphore>& presentSemaphores,
            std::vector<VkSemaphore>& renderSemaphores,
            std::vector<VkFence>& inFlightFences);

        void cleanupSynchronization(VkDevice device, std::vector<VkSemaphore>& presentSemaphores,
            std::vector<VkSemaphore>& renderSemaphores, std::vector<VkFence>& inFlightFences);
    }

}


#endif // !INCLUDE_SOURCE_HELPER_H_