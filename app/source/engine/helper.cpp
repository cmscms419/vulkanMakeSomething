#include "helper.h"

namespace vkengine {
    namespace helper {
        std::vector<char> readFile(const std::string& filename)
        {
            // ���� ������ �̵��Ͽ� ���� ũ�⸦ �����ɴϴ�.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // ������ �� �� ���� ��� ���ܸ� �߻���ŵ�ϴ�.
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // ���� ũ�⸦ �̿��Ͽ� ���۸� �Ҵ��մϴ�.
            std::vector<char> buffer(fileSize);     // ���� �����͸� ������ �������� �̵��մϴ�.
            file.seekg(0);                          // ���� �����͸� ������ �������� �̵��մϴ�.
            file.read(buffer.data(), fileSize);     // ���� ������ ���ۿ� �о�ɴϴ�. -> ������ fileSize ũ�⸸ŭ �ѹ��� �о�´�.
            file.close();                           // ������ �ݽ��ϴ�.

            return buffer;
        }

        bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices)
        {
            QueueFamilyIndices indices_ = findQueueFamilies(device, VKsurface);
            indices = indices_;

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

        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice &device, VkSurfaceKHR &VKsurface)
        {
            QueueFamilyIndices indices; // ť �йи��� ������ ������ ������ �ʱ�ȭ
            QueueFamilyIndices target; // ť �йи��� ������ ������ ������ �ʱ�ȭ

            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (ù ��° ȣ���� ������ ������)
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            
            // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ������ (�� ��° ȣ���� ���� �Ӽ��� ������)
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            
#ifdef DEBUG_
            getPyhsicalDeviceProperties(device);
#endif // DEBUG_

            int i = 0;
            bool selected = false;
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
                    PRINT_TO_CONSOLE("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    PRINT_TO_CONSOLE("VK_QUEUE_COMPUTE_BIT is supported\n");
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
                    PRINT_TO_CONSOLE("VK_QUEUE_PRESENT_BIT is supported\n");
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
                        PRINT_TO_CONSOLE("------------------ select Queuefamily index: %d ------------------\n", i);
#endif // DEBUG
                        selected = true;
                    }
                }

                indices.reset();
                i++;
            }

            return target;
        }

        void createImage(VkDevice VKdevice, VkPhysicalDevice VKphysicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
        {
            VkImageCreateInfo imageInfo{};

            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = numSamples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            
            VK_CHECK_RESULT(vkCreateImage(VKdevice, &imageInfo, nullptr, &image));
            
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(VKdevice, image, &memRequirements);
            
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);
            
            VK_CHECK_RESULT(vkAllocateMemory(VKdevice, &allocInfo, nullptr, &imageMemory));
            
            vkBindImageMemory(VKdevice, image, imageMemory, 0);
        }

        void copyBuffer(VkDevice VKdevice, VkCommandPool VKcommandPool, VkQueue graphicsVKQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(VKdevice, VKcommandPool);
            
            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
            
            endSingleTimeCommands(VKdevice, VKcommandPool, graphicsVKQueue, commandBuffer);
        }

        void copyBuffer2(vkengine::VKDevice_& VKdevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(VKdevice.logicaldevice, VKdevice.commandPool);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(VKdevice.logicaldevice, VKdevice.commandPool, VKdevice.graphicsVKQueue, commandBuffer);
        }

        void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
        {
            // ���� ���� ������ ���� ����ü�� �ʱ�ȭ�Ѵ�.
            VkBufferCreateInfo bufferInfo{};
            
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;        // ����ü Ÿ���� �����Ѵ�.
            bufferInfo.size = size;                                         // ������ ������ ũ�⸦ �����Ѵ�.
            bufferInfo.usage = usage;                                       // ���� ��� ������ �����Ѵ� (��: vertex, index ��).
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;             // ������ ���� ��带 �������� �����Ѵ�.

            // ������ ������ �������� ���۸� �����ϰ�, ���� ���θ� �˻��Ѵ�.
            VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

            // ������ ���ۿ� �ʿ��� �޸� �䱸���� ������ �����´�.
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            // �޸� �Ҵ� ������ ���� ����ü�� �ʱ�ȭ�Ѵ�.
            VkMemoryAllocateInfo allocInfo{};
            
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;                                                   // ����ü Ÿ���� �����Ѵ�.    
            allocInfo.allocationSize = memRequirements.size;                                                            // ���۸� ���� �޸� ũ�⸦ �����Ѵ�.
            allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);     // �䱸���׿� �´� �޸� Ÿ�� �ε����� ã�Ƽ� �����Ѵ�.
            
            VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));  // �� ������ �������� �޸𸮸� �Ҵ��ϰ� ���� ���θ� �˻��Ѵ�.
            VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));           // �Ҵ�� �޸𸮸� ���ۿ� ���ε��Ͽ� GPU���� ����� �� �ְ� �Ѵ�.
        }

        void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { width, height, 1 };

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
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

#ifdef DEBUG_
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

        VkFormat findSupportedFormat(
            VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features)
        {
            VkFormat value{ VK_FORMAT_UNDEFINED };

            for (VkFormat format : candidates)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                {
                    value = format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                {
                    value = format;
                }
                else
                {
                    PRINT_TO_CONSOLE("Failed to find supported format!\n");
                }
            }

            return value;
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

        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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

            VkImageView imageView{VK_NULL_HANDLE};

            VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

            return imageView;
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

        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
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
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
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

        /**
            * @brief �̹����� ���̾ƿ��� ��ȯ�ϴ� �Լ�
            *
            * �� �Լ��� ������ �̹����� ���� �ʿ信 ���� �̹��� �޸� �踮� �����ϰ�,
            * ���� ���̾ƿ�(oldLayout)���� ���ο� ���̾ƿ�(newLayout)���� �̹����� �����մϴ�.
            * �̹��� ��ȯ�� ���� �ð� ��� ���۸� ����Ͽ� ����Ǹ�, ��ȯ �۾� �� ���� ��� ���۴�
            * ����Ǿ� �Ϸ�� ������ ���˴ϴ�.
            * -> ���� ���Ŀ��� �ٸ� �������� �̹����� ����ϰ� �ٰ� �� �� ����Ѵ�.
            *
            * @remark �����Ǵ� ��ȯ�� �Ʒ��� 3���� ����Դϴ�:
            *  1) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            *  2) VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            *  3) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            *
         */
        void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
        {
            // ���� �ð� ��� ���۸� �����մϴ�.
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // �̹��� �޸� �踮�� ����ü�� �ʱ�ȭ�Ͽ� ���̾ƿ� ��ȯ �� ���� ���� ������ �����մϴ�.
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // �踮���� ����ü Ÿ�� ����
            barrier.oldLayout = oldLayout;                          // ��ȯ �� �̹��� ���̾ƿ�
            barrier.newLayout = newLayout;                          // ��ȯ�� �̹��� ���̾ƿ�
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // �ҽ� ť �йи� �ε��� ����
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // ��� ť �йи� �ε��� ����

            barrier.image = image; // ��ȯ�� �̹��� �ڵ� ����
            // �⺻������ ���� ������ ������� ������, ���� ���ǿ� ���� �����˴ϴ�.
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // ù ��° ������ �������� ����
            barrier.subresourceRange.levelCount = mipLevels; // ������ �������� ��
            barrier.subresourceRange.baseArrayLayer = 0;     // ù ��° �迭 ���̾���� ����
            barrier.subresourceRange.layerCount = 1;         // �迭 �� ���̾� ����

            VkPipelineStageFlags sourceStage;      // ��ȯ �� ���������� ��������
            VkPipelineStageFlags destinationStage; // ��ȯ �� ���������� ��������

            // ���ο� ���̾ƿ��� ����/���ٽǿ��� ��� aspectMask�� �����մϴ�.
            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                // ���˿� ���ٽ� ������Ʈ�� �ִٸ� �߰��մϴ�.
                if (hasStencilComponent(format))
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            // ��ȯ Ÿ�Կ� ���� �׼��� ����ũ �� ���������� �������� ����
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                // ���� ���̾ƿ��� ���ǵ��� ���� �����̸� � �׼����� ������� �ʽ��ϴ�.
                barrier.srcAccessMask = 0;
                // ���� ���� �۾��� ���� ���� �׼��� ��� ����
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                // ���� �Ϸ� �� ���̴� �б� �������� ��ȯ
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                // ����/���ٽ� �̹��� �ʱ�ȭ ����, ���� ���̾ƿ��� ��� �Ұ����� ���¿��� ������
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else {
                // �������� �ʴ� ���̾ƿ� ��ȯ ��û�� ���� �߻�
                PRINT_TO_CONSOLE("Unsupported layout transition!\n");
                return;
            }

            // ���������� �踮� �߰��Ͽ� ���̾ƿ� ��ȯ ����� ����մϴ�.
            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage, // ��ȯ ��/�� ��������
                0,                // �踮�� �÷��� (������� ����)
                0, nullptr,     // �޸� �踮�� ����
                0, nullptr,     // ���� �踮�� ����
                1, &barrier     // �ϳ��� �̹��� �踮�� ���
            );

            // ���� ��� ���۸� �����ϰ�, ť�� �ش� �۾��� �Ϸ��� ������ ����մϴ�.
            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
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