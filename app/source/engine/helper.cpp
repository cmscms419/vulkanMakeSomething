#include "helper.h"

namespace vkengine {
    namespace helper {
        std::vector<char> readFile(const std::string& filename)
        {
            // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // 파일을 열 수 없는 경우 예외를 발생시킵니다.
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
            std::vector<char> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
            file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
            file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
            file.close();                           // 파일을 닫습니다.

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
            QueueFamilyIndices indices; // 큐 패밀리의 개수를 저장할 변수를 초기화
            QueueFamilyIndices target; // 큐 패밀리의 개수를 저장할 변수를 초기화

            // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (첫 번째 호출은 개수만 가져옴)
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            
            // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (두 번째 호출은 실제 속성을 가져옴)
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
            
#ifdef DEBUG_
            getPyhsicalDeviceProperties(device);
#endif // DEBUG_

            int i = 0;
            bool selected = false;
            for (const auto& queueFamily : queueFamilies) {
                // 현재 큐 패밀리가 그래픽스 큐를 지원하는지 확인
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
            // 버퍼 생성 정보를 담은 구조체를 초기화한다.
            VkBufferCreateInfo bufferInfo{};
            
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;        // 구조체 타입을 지정한다.
            bufferInfo.size = size;                                         // 생성할 버퍼의 크기를 설정한다.
            bufferInfo.usage = usage;                                       // 버퍼 사용 목적을 지정한다 (예: vertex, index 등).
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;             // 버퍼의 공유 모드를 독점으로 설정한다.

            // 지정된 정보를 바탕으로 버퍼를 생성하고, 성공 여부를 검사한다.
            VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

            // 생성된 버퍼에 필요한 메모리 요구사항 정보를 가져온다.
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            // 메모리 할당 정보를 담은 구조체를 초기화한다.
            VkMemoryAllocateInfo allocInfo{};
            
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;                                                   // 구조체 타입을 지정한다.    
            allocInfo.allocationSize = memRequirements.size;                                                            // 버퍼를 위한 메모리 크기를 설정한다.
            allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);     // 요구사항에 맞는 메모리 타입 인덱스를 찾아서 지정한다.
            
            VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));  // 위 정보를 바탕으로 메모리를 할당하고 성공 여부를 검사한다.
            VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, bufferMemory, 0));           // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
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
                { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },    // 후보 형식
                VK_IMAGE_TILING_OPTIMAL,                                                                // 타일링
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT                                          // 특징
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
            VkCommandBufferAllocateInfo allocInfo{};                          // 커맨드 버퍼 할당 정보 구조체를 초기화합니다.
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // 구조체 타입을 설정합니다.
            
            // 커맨드 버퍼 레벨을 설정합니다.
            // VK_COMMAND_BUFFER_LEVEL_PRIMARY: 기본 커맨드 버퍼
            // VK_COMMAND_BUFFER_LEVEL_SECONDARY: 보조 커맨드 버퍼
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;                              // 커맨드 풀을 설정합니다.
            allocInfo.commandBufferCount = 1;                                 // 커맨드 버퍼 개수를 설정합니다.

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);     // 커맨드 버퍼를 할당합니다.

            VkCommandBufferBeginInfo beginInfo{};
            // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO : 명령 버퍼의 시작 정보를 설정합니다.
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            
            // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 커맨드 버퍼를 한 번만 사용하려는 경우 사용합니다.
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            // 커맨드 버퍼를 시작합니다.
            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;

        }
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
        {   
            // 커맨드 버퍼를 종료합니다.
            vkEndCommandBuffer(commandBuffer);
            
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1; // 커맨드 버퍼 개수를 설정합니다.
            submitInfo.pCommandBuffers = &commandBuffer; // 커맨드 버퍼를 설정합니다.
            
            // 큐에 커맨드 버퍼를 제출합니다.
            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            
            // 큐가 모든 작업을 완료할 때까지 대기합니다.
            vkQueueWaitIdle(graphicsQueue);
            
            // 커맨드 버퍼를 해제합니다.
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        /**
            * @brief 이미지의 레이아웃을 전환하는 함수
            *
            * 이 함수는 지정된 이미지에 대해 필요에 따른 이미지 메모리 배리어를 설정하고,
            * 기존 레이아웃(oldLayout)에서 새로운 레이아웃(newLayout)으로 이미지를 변경합니다.
            * 이미지 전환은 단일 시간 명령 버퍼를 사용하여 수행되며, 전환 작업 후 관련 명령 버퍼는
            * 제출되어 완료될 때까지 대기됩니다.
            * -> 기존 형식에서 다른 형식으로 이미지를 사용하게 다고 할 때 사용한다.
            *
            * @remark 지원되는 전환은 아래의 3가지 경우입니다:
            *  1) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            *  2) VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL -> VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            *  3) VK_IMAGE_LAYOUT_UNDEFINED -> VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            *
         */
        void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
        {
            // 단일 시간 명령 버퍼를 시작합니다.
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // 이미지 메모리 배리어 구조체를 초기화하여 레이아웃 전환 및 접근 권한 변경을 정의합니다.
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // 배리어의 구조체 타입 설정
            barrier.oldLayout = oldLayout;                          // 전환 전 이미지 레이아웃
            barrier.newLayout = newLayout;                          // 전환할 이미지 레이아웃
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시

            barrier.image = image; // 전환할 이미지 핸들 설정
            // 기본적으로 색상 정보를 대상으로 하지만, 이후 조건에 따라 수정됩니다.
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // 첫 번째 미프맵 레벨부터 시작
            barrier.subresourceRange.levelCount = mipLevels; // 적용할 미프맵의 수
            barrier.subresourceRange.baseArrayLayer = 0;     // 첫 번째 배열 레이어부터 시작
            barrier.subresourceRange.layerCount = 1;         // 배열 내 레이어 개수

            VkPipelineStageFlags sourceStage;      // 전환 전 파이프라인 스테이지
            VkPipelineStageFlags destinationStage; // 전환 후 파이프라인 스테이지

            // 새로운 레이아웃이 깊이/스텐실용일 경우 aspectMask를 수정합니다.
            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                // 포맷에 스텐실 컴포넌트가 있다면 추가합니다.
                if (hasStencilComponent(format))
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            // 전환 타입에 따른 액세스 마스크 및 파이프라인 스테이지 설정
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                barrier.srcAccessMask = 0;
                // 전송 쓰기 작업을 위한 쓰기 액세스 허용 설정
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                // 깊이/스텐실 이미지 초기화 경우로, 이전 레이아웃이 사용 불가능한 상태에서 시작함
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else {
                // 지원되지 않는 레이아웃 전환 요청시 예외 발생
                PRINT_TO_CONSOLE("Unsupported layout transition!\n");
                return;
            }

            // 파이프라인 배리어를 추가하여 레이아웃 전환 명령을 기록합니다.
            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage, // 전환 전/후 스테이지
                0,                // 배리어 플래그 (사용하지 않음)
                0, nullptr,     // 메모리 배리어 없이
                0, nullptr,     // 버퍼 배리어 없이
                1, &barrier     // 하나의 이미지 배리어 사용
            );

            // 단일 명령 버퍼를 제출하고, 큐가 해당 작업을 완료할 때까지 대기합니다.
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