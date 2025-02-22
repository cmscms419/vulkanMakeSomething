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
            QueueFamilyIndices target; // ť �йи��� ������ ������ ������ �ʱ�ȭ

            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (ù ��° ȣ���� ������ ������)
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            
            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (�� ��° ȣ���� ���� �Ӽ��� ������)
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            
            int i = 0;
            bool selected = false;

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
                    if (!selected)
                    {
                        indices.setgraphicsAndComputeFamily(i);
                    }
                }
                
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKsurface, &presentSupport);

                if (presentSupport)
                {
#ifdef DEBUG_
                    printf("VK_QUEUE_PRESENT_BIT is supported\n");
#endif // DEBUG
                    if (!selected)
                    {
                        indices.setPresentFamily(i);
                    }
                }

                if (indices.isComplete()) {
                    
                    if (!selected)
                    {
                        indices.queueFamilyProperties = queueFamily;
                        target = indices;
#ifdef DEBUG_
                        printf("------------------ select Queuefamily index: %d ------------------\n", i);
#endif // DEBUG
                        selected = true;
                    }
                }

                indices.reset();
                i++;
            }

            return target;
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

        uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            return 0;
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
        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.pNext = nullptr;
            info.queueFamilyIndex = queueFamilyIndex;
            info.flags = flags;
            return info;
        }
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
        {
            VkCommandBufferAllocateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.pNext = nullptr;
            info.commandPool = pool;
            info.commandBufferCount = count;
            info.level = level;
            
            return info;
        }
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags)
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;

            info.flags = flags;

            return info;
        }

        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags)
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = flags;
            return info;
        }

        VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool)
        {
            VkCommandBufferAllocateInfo allocInfo{};                          // Ŀ�ǵ� ���� �Ҵ� ���� ����ü�� �ʱ�ȭ�մϴ�.
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // ����ü Ÿ���� �����մϴ�.
            
            // Ŀ�ǵ� ���� ������ �����մϴ�.
            // VK_COMMAND_BUFFER_LEVEL_PRIMARY: �⺻ Ŀ�ǵ� ����
            // VK_COMMAND_BUFFER_LEVEL_SECONDARY: ���� Ŀ�ǵ� ����
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;                              // Ŀ�ǵ� Ǯ�� �����մϴ�.
            allocInfo.commandBufferCount = 1;                                 // Ŀ�ǵ� ���� ������ �����մϴ�.

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);     // Ŀ�ǵ� ���۸� �Ҵ��մϴ�.

            VkCommandBufferBeginInfo beginInfo{};
            // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO : ��� ������ ���� ������ �����մϴ�.
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
            // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : Ŀ�ǵ� ���۸� �� ���� ����Ϸ��� ��� ����մϴ�.
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            // Ŀ�ǵ� ���۸� �����մϴ�.
            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;

        }
        void endSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue, VkCommandBuffer& commandBuffer)
        {   
            // Ŀ�ǵ� ���۸� �����մϴ�.
            vkEndCommandBuffer(commandBuffer);
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1; // Ŀ�ǵ� ���� ������ �����մϴ�.
            submitInfo.pCommandBuffers = &commandBuffer; // Ŀ�ǵ� ���۸� �����մϴ�.
            
            // ť�� Ŀ�ǵ� ���۸� �����մϴ�.
            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            
            // ť�� ��� �۾��� �Ϸ��� ������ ����մϴ�.
            vkQueueWaitIdle(graphicsQueue);
            
            // Ŀ�ǵ� ���۸� �����մϴ�.
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
        {
            // Ŀ�ǵ� ���۸� �����մϴ�.
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // �̹��� �޸� �踮� �����մϴ�.
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // �̹��� �޸� �踮���� ����ü Ÿ���� �����մϴ�.
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.image = image;                                           // �̹����� �����մϴ�.
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // �̹����� aspectMask(���� ��Ʈ)�� �����մϴ�.
            barrier.subresourceRange.baseMipLevel = 0;                       // �̹����� �⺻ ������ ������ �����մϴ�.
            barrier.subresourceRange.levelCount = mipLevels;                 // �̹����� ������ ���� ������ �����մϴ�.
            barrier.subresourceRange.baseArrayLayer = 0;                     // �̹����� �⺻ �迭 ���̾ �����մϴ�.
            barrier.subresourceRange.layerCount = 1;                         // �̹����� ���̾� ������ �����մϴ�.

            VkPipelineStageFlags sourceStage;      // �ҽ� ���������� �����մϴ�.
            VkPipelineStageFlags destinationStage; // ��� ���������� �����մϴ�.

            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                if (hasStencilComponent(format)) 
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else {
                throw std::invalid_argument("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

        }
        
        bool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
        {
            VkFenceCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;

            info.flags = flags;

            return info;
        }

        VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
        {
            VkSemaphoreCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = flags;
            return info;
        }

    }
}