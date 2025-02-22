#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include "../_common.h"

namespace vkengine {
    namespace helper {

        // ���� ����̽��� �䱸 ������ �����ϴ��� Ȯ���ϴ� �Լ�
        bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices* indices);

        // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ã�� �Լ�
        // PROB : ť �йи��� �������� ��쿡 �ʿ��� ó���� �ִ� �йи��� ���� ã�� ���, �� �йи��� �ε����� ��ȯ��
        // TODO ; ť �йи��� �������� ��쿡 ���� ó���� �ʿ���
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

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
            VkDevice& device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels);
        
        // ���ٽ� ������Ʈ�� ������ �ִ��� Ȯ���ϴ� �Լ�
        bool hasStencilComponent(VkFormat format);
        
        // Mipmaps�� �����ϴ� �Լ�
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

        // �ִ� ��� ������ ���ø� ���� ��ȯ�ϴ� �Լ�
        
        VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
        
        // setupCommandBuffer ���߿� �߰�
        // flushSetupCommands ���߿� �߰�

        // pyhsicalDeviceProperties�� �������� �Լ�
        void getPyhsicalDeviceProperties(VkPhysicalDevice device);

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
        VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool);

        // ��ɹ��۸� �����ϴ� �Լ�
        void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkCommandBuffer& commandBuffer);

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

        // �̹��� ���̾ƿ��� ��ȯ�ϴ� �Լ�(
        //void 

        // stencilComponent�� ������ �ִ��� Ȯ���ϴ� �Լ�
        bool hasStencilComponent(VkFormat format);

    }
}

#endif // !INCLUDE_SOURCE_HELPER_H_
