#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include <set>
#include <fstream>

#include "../../common/common.h"
#include "../../common/struct.h"
#include "../../common/macros.h"

#include "VKbuffer.h"
#include "VKdevice.h"

namespace vkengine {
    struct VKDevice_;

    namespace helper {

        // ������ �о���� �Լ�
        std::vector<char> readFile(const std::string& filename);

        // ���� ����̽��� �䱸 ������ �����ϴ��� Ȯ���ϴ� �Լ�
        bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices);

        // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ã�� �Լ�
        // PROB : ť �йи��� �������� ��쿡 �ʿ��� ó���� �ִ� �йи��� ���� ã�� ���, �� �йи��� �ε����� ��ȯ��
        // TODO ; ť �йи��� �������� ��쿡 ���� ó���� �ʿ���
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface);

        void createImage(
            VkDevice VKdevice,
            VkPhysicalDevice VKphysicalDevice,
            uint32_t width,
            uint32_t height,
            uint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory,
            uint32_t arrayLayer = 1);


        // ���۸� �����ϴ� �Լ�
        void copyBuffer(
            VkDevice VKdevice,
            VkCommandPool VKcommandPool,
            VkQueue graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        void copyBuffer2(
            vkengine::VKDevice_& VKdevice,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        // buffer�� �����ϴ� �Լ�
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
            uint32_t width,
            uint32_t height);

        void copyBufferToImage2(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height,
            std::vector<VkDeviceSize> &sizeArray);

        // ���� ����̽��� Ȯ�� ����� �����ϴ��� Ȯ���ϴ� �Լ�
        // ���� ���̾� ���� ���θ� Ȯ���ϴ� �Լ�
        // Ȯ�� ����� �����ϰ� �ʿ��� ��� Ȯ�� ����� ���ԵǾ� �ִ��� Ȯ��
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // ���� ����̽��� ��� Ȯ������ �������� �Լ�
        void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<std::string>* temp);

        // ���� ����̽��� ���ռ��� ���ϴ� �Լ�
        int rateDeviceSuitability(VkPhysicalDevice device);

        // ���� ü�� ���� ������ �������� �Լ�
        // ���� ü�� ���� ������ ������ ����ü�� �ʱ�ȭ
        // ���� ��ġ���� ���ǽ��� ����� ������
        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

        // ���� ������ ã�� �Լ�
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

        // Format�� �����ϴ��� Ȯ���ϴ� �Լ�
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
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels,
            uint32_t imageCount = 1);

        VkImageView createArrayImageView(
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels,
            uint32_t imageCount);

        // ���ٽ� ������Ʈ�� ������ �ִ��� Ȯ���ϴ� �Լ�
        inline bool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        // Mipmaps�� �����ϴ� �Լ�
        /*void generateMipmaps(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat imageFormat,
            int32_t texWidth,
            int32_t texHeight,
            uint32_t mipLevels);*/

        // �ִ� ��� ������ ���ø� ���� ��ȯ�ϴ� �Լ�

        //VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer ���߿� �߰�
        // flushSetupCommands ���߿� �߰�

        // pyhsicalDeviceProperties�� �������� �Լ�
        inline void getPyhsicalDeviceProperties(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            _PRINT_TO_CONSOLE_("Device Name: %s\n", deviceProperties.deviceName);
            _PRINT_TO_CONSOLE_("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        }

        // commandPood�� ���� information�� �����ϴ� �Լ�
        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

        // commandBuffer�� ���� information�� �����ϴ� �Լ�
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Fencee�� ���� information�� �����ϴ� �Լ�
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

        // Semaphore�� ���� information�� �����ϴ� �Լ�
        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

        // https://steel-gourd-618.notion.site/Images-19618a41dc6f80b89bc1d1575bcf3d04 ����
        //  �����Ϸ��� ��ɹ��۸� �����ϴ� �Լ�
        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

        // ��ɹ��۸� �����ϴ� �Լ�
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);

        // �̹��� ���̾ƿ��� ��ȯ�ϴ� �Լ�
        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels);

        // �̹����迭 ���̾ƿ��� ��ȯ�ϴ� �Լ�
        void transitionImageLayout2(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels,
            uint32_t layerCount);

        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkFormat format,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange
        );

        // stencilComponent�� ������ �ִ��� Ȯ���ϴ� �Լ�
        bool hasStencilComponent(VkFormat format);

        // Ư�� �����͸� VkDeviceMemory ��ü�� �����ϴ� �Լ�
        template <typename T>
        inline void copyToDeviceMemory(VkDevice device, const T* src, VkDeviceMemory dst, VkDeviceSize size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0)
        {
            void* data;
            vkMapMemory(device, dst, offset, size, flags, &data);
            memcpy(data, src, (size_t)size);
            vkUnmapMemory(device, dst);
        }

        inline VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;

            info.flags = flags;

            return info;
        }

        inline VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = flags;
            return info;
        }

    }

}


#endif // !INCLUDE_SOURCE_HELPER_H_