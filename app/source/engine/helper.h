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

    }
}

#endif // !INCLUDE_SOURCE_HELPER_H_
