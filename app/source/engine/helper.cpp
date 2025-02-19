#include "helper.h"

namespace vkengine {
    namespace helper {

        bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices* indices)
        {
            QueueFamilyIndices indices_ = findQueueFamilies(device, VKsurface);
            *indices = indices_;

            bool extensionsSupported = checkDeviceExtensionSupport(device);
            bool swapChainAdequate = false;

            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, VKsurface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }

        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR VKsurface)
        {
            QueueFamilyIndices indices; // ť �йи��� ������ ������ ������ �ʱ�ȭ

            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (ù ��° ȣ���� ������ ������)
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            
            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (�� ��° ȣ���� ���� �Ӽ��� ������)
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            
            int i = 0;

            getPyhsicalDeviceProperties(device);

            for (const auto& queueFamily : queueFamilies) {
                // ���� ť �йи��� �׷��Ƚ� ť�� �����ϴ��� Ȯ��
#ifdef DEBUG_
                printf("QueueFamily %d\n", i);
                printf("QueueFamily queueCount: %d\n", queueFamily.queueCount);
                printf("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
                printf("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                printf("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                printf("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                printf("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);
#endif // DEBUG_

                if ((queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                    && (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)) {
#ifdef DEBUG_
                    printf("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    printf("VK_QUEUE_COMPUTE_BIT is supported\n");
#endif // DEBUG_
                    indices.setgraphicsAndComputeFamily(i);
                }
                
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKsurface, &presentSupport);

                if (presentSupport)
                {
#ifdef DEBUG_
                    printf("VK_QUEUE_PRESENT_BIT is supported\n");
#endif // DEBUG
                    indices.setPresentFamily(i);
                }

                if (indices.isComplete()) {
                    indices.queueFamilyProperties = queueFamily;
#ifdef DEBUG_
                    printf("Queuefamily index: %d\n", i);
                    printf("\n");
#endif // DEBUG

                    break;
                }

                indices.reset();
                i++;
            }

            return indices;
        }

        bool checkDeviceExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {

#ifdef DEBUG_
                std::cout << '\t' << extension.extensionName << '\n';
#endif // DEBUG

                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<std::string>* temp)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            for (const auto& extension : availableExtensions) {
                temp->insert(extension.extensionName);
            }
        }

        int rateDeviceSuitability(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            int score = 0;

            // Discrete GPUs have a significant performance advantage
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }

            // Maximum possible size of textures affects graphics quality
            score += deviceProperties.limits.maxImageDimension2D;

            // Application can't function without geometry shaders
            if (!deviceFeatures.geometryShader) {
                return 0;
            }

#ifdef DEBUG_/
            printf("Device %s score: %d\n", deviceProperties.deviceName, score);
            printf("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
            printf("Device Name: %s\n", deviceProperties.deviceName);
            printf("\n");
#endif // DEBUG_

            return score;
        }

        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VKsurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
        {
            return findSupportedFormat(
                physicalDevice,
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },    // �ĺ� ����
                VK_IMAGE_TILING_OPTIMAL,                                                                // Ÿ�ϸ�
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT                                          // Ư¡
            );
        }

        VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
        {
            for (VkFormat format : candidates)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                {
                    return format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                {
                    return format;
                }
            }
            throw std::runtime_error("failed to find supported format!");
        }

        VkImageView createImageView(VkDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }

            return imageView;
        }

        void getPyhsicalDeviceProperties(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
#ifdef DEBUG_
            printf("Device Name: %s\n", deviceProperties.deviceName);
            printf("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
#endif // DEBUG_

        }
    }
}