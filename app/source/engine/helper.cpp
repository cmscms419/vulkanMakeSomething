#include "helper.h"

namespace vkengine {
    namespace helper {

        VkAccessFlags getFromOldLayoutToVkAccessFlags(VkImageLayout format)
        {

            VkAccessFlags accessFlags = 0;

            switch (format)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                // or 깊이/스텐실 이미지 초기화 경우로, 이전 레이아웃이 사용 불가능한 상태에서 시작함
                accessFlags = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

            return accessFlags;
        }
        VkAccessFlags getFromNewLayoutToVkAccessFlags(VkImageLayout format)
        {

            VkAccessFlags accessFlags = 0;

            switch (format)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                accessFlags = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // 전송 완료 후 색상 첨부 최적화 레이아웃으로 전환
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 쓰기 작업을 위한 쓰기 액세스 허용 설정
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }
            return accessFlags;
        }

        std::vector<cChar> readFile(const std::string& filename)
        {
            // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // 파일을 열 수 없는 경우 예외를 발생시킵니다.
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
            std::vector<cChar> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
            file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
            file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
            file.close();                           // 파일을 닫습니다.

            return buffer;
        }

        bool fileExists(const std::string& filename)
        {
            std::ifstream f(filename.c_str());
            return !f.fail();
        }

        uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        size_t alignedSize(size_t value, size_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }


        VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        cBool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices)
        {
            QueueFamilyIndices indices_ = findQueueFamilies(device, VKsurface);
            indices = indices_;

            cBool extensionsSupported = checkDeviceExtensionSupport(device);
            cBool swapChainAdequate = false;

            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, VKsurface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }

        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface)
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
            cBool selected = false;
            for (const auto& queueFamily : queueFamilies) {
                // 현재 큐 패밀리가 그래픽스 큐를 지원하는지 확인
                _PRINT_TO_CONSOLE_("QueueFamily %d\n", i);
                _PRINT_TO_CONSOLE_("QueueFamily queueCount: %d\n", queueFamily.queueCount);
                _PRINT_TO_CONSOLE_("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
                _PRINT_TO_CONSOLE_("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                _PRINT_TO_CONSOLE_("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                _PRINT_TO_CONSOLE_("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                _PRINT_TO_CONSOLE_("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);

                if ((queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                    && (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)) {
                    _PRINT_TO_CONSOLE_("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    _PRINT_TO_CONSOLE_("VK_QUEUE_COMPUTE_BIT is supported\n");
                    if (!selected)
                    {
                        indices.setgraphicsAndComputeFamily(i);
                    }
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKsurface, &presentSupport);

                if (presentSupport)
                {
                    _PRINT_TO_CONSOLE_("VK_QUEUE_PRESENT_BIT is supported\n");
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
                        _PRINT_TO_CONSOLE_("------------------ select Queuefamily index: %d ------------------\n", i);
                        selected = true;
                    }
                }

                indices.reset();
                i++;
            }

            return target;
        }

        void createImage(VkDevice VKdevice, VkPhysicalDevice VKphysicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t arrayLayer)
        {
            VkImageCreateInfo imageInfo{};

            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = { width, height, 1 };
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = arrayLayer;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = numSamples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            _VK_CHECK_RESULT_(vkCreateImage(VKdevice, &imageInfo, nullptr, &image));

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(VKdevice, image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);

            _VK_CHECK_RESULT_(vkAllocateMemory(VKdevice, &allocInfo, nullptr, &imageMemory));
            _VK_CHECK_RESULT_(vkBindImageMemory(VKdevice, image, imageMemory, 0));
        }

        void createImage2(VkDevice VKdevice, VkPhysicalDevice VKphysicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t arrayLayer, VkImageCreateFlagBits flag)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = { width, height, 1 };
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = arrayLayer;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = numSamples;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (flag == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
                imageInfo.flags = flag;
            else
                imageInfo.flags = 0;

            _VK_CHECK_RESULT_(vkCreateImage(VKdevice, &imageInfo, nullptr, &image));
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(VKdevice, image, &memRequirements);
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);
            _VK_CHECK_RESULT_(vkAllocateMemory(VKdevice, &allocInfo, nullptr, &imageMemory));
            _VK_CHECK_RESULT_(vkBindImageMemory(VKdevice, image, imageMemory, 0));
        }

        void copyBuffer(VkDevice VKdevice, VkCommandPool VKcommandPool, VkQueue graphicsVKQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(VKdevice, VKcommandPool);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(VKdevice, VKcommandPool, graphicsVKQueue, commandBuffer);
        }

        void copyBuffer2(vkengine::VKdeviceHandler& VKdevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
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

            // 지정된 정보를 바탕으로 버퍼를 생성
            _VK_CHECK_RESULT_(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

            // 생성된 버퍼에 필요한 메모리 요구사항 정보를 가져온다.
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            // 메모리 할당 정보를 담은 구조체를 초기화한다.
            VkMemoryAllocateInfo allocInfo{};

            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;                                                   // 구조체 타입을 지정한다.    
            allocInfo.allocationSize = memRequirements.size;                                                            // 버퍼를 위한 메모리 크기를 설정한다.
            allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);     // 요구사항에 맞는 메모리 타입 인덱스를 찾아서 지정한다.

            _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));  // 위 정보를 바탕으로 메모리를 할당하고 성공 여부를 검사한다.
            _VK_CHECK_RESULT_(vkBindBufferMemory(device, buffer, bufferMemory, 0));           // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
        }

        void copyBufferToImage(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height)
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

            // buffer의 데이터를 image로 복사한다.
            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void copyBufferToImage2(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::vector<VkDeviceSize>& sizeArray)
        {

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // VKBufferImageCopy 배열을 만든다.
            std::vector<VkBufferImageCopy> bufferCopyRegions;
            VkDeviceSize offset = 0;

            for (uint32_t i = 0; i < sizeArray.size(); i++)
            {
                VkBufferImageCopy region{};

                region.bufferOffset = offset;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = i;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { width, height, 1 };

                bufferCopyRegions.push_back(region);
                offset += sizeArray[i];
            }

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void copyBufferToImageKTX(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipmapLevels, ktxTexture* textureKTX)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // VKBufferImageCopy 배열을 만든다.
            std::vector<VkBufferImageCopy> bufferCopyRegions;

            for (uint32_t face = 0; face < 6; face++)
            {
                for (uint32_t level = 0; level < mipmapLevels; level++)
                {
                    VkDeviceSize offset = 0;
                    KTX_error_code result = ktxTexture_GetImageOffset(textureKTX, level, 0, face, &offset);
                    _CHECK_RESULT_((result == KTX_SUCCESS));

                    VkBufferImageCopy bufferCopyRegion = {};
                    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    bufferCopyRegion.imageSubresource.mipLevel = level;
                    bufferCopyRegion.imageSubresource.baseArrayLayer = face;
                    bufferCopyRegion.imageSubresource.layerCount = 1;
                    bufferCopyRegion.imageExtent.width = textureKTX->baseWidth >> level;
                    bufferCopyRegion.imageExtent.height = textureKTX->baseHeight >> level;
                    bufferCopyRegion.imageExtent.depth = 1;
                    bufferCopyRegion.bufferOffset = offset;

                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        cBool checkDeviceExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {

                _PRINT_TO_CONSOLE_("Available Extension: %s\n", extension.extensionName);
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

            _PRINT_TO_CONSOLE_("Device %s score: %d\n", deviceProperties.deviceName, score);
            _PRINT_TO_CONSOLE_("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
            _PRINT_TO_CONSOLE_("Device Name: %s\n", deviceProperties.deviceName);
            _PRINT_TO_CONSOLE_("\n");

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
                    _PRINT_TO_CONSOLE_("Failed to find supported format!\n");
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

        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, cSize imageCount)
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
            viewInfo.subresourceRange.layerCount = static_cast<cUint32_t>(imageCount);

            VkImageView imageView{ VK_NULL_HANDLE };

            _VK_CHECK_RESULT_(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

            return imageView;
        }

        VkImageView createArrayImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, cSize imageCount)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = static_cast<cUint32_t>(imageCount);

            VkImageView imageView{ VK_NULL_HANDLE };

            _VK_CHECK_RESULT_(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

            return imageView;
        }

        VkImageView createCubeImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = mipLevels;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 6;

            VkImageView imageView{ VK_NULL_HANDLE };

            _VK_CHECK_RESULT_(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

            return imageView;
        }

        void generateMipmaps(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
        {
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

            _CHECK_RESULT_((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++) {

                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                VkImageBlit blit{};

                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;

                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                if (mipWidth > 1)
                {
                    mipWidth /= 2;
                }

                if (mipHeight > 1)
                {
                    mipHeight /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

        }

        void generateMipmapsCubeMap(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
        {
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

            _CHECK_RESULT_((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT));

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
            VkImageMemoryBarrier barrier{};

            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1; // Cube map has 6 layers
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++)
            {
                for (uint32_t face = 0; face < 6; face++)
                {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.subresourceRange.baseArrayLayer = face;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    VkImageBlit blit{};

                    blit.srcOffsets[0] = { 0, 0, 0 };
                    blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.srcSubresource.baseArrayLayer = face;
                    blit.srcSubresource.layerCount = 1;

                    blit.dstOffsets[0] = { 0, 0, 0 };
                    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = i;
                    blit.dstSubresource.baseArrayLayer = face;
                    blit.dstSubresource.layerCount = 1;

                    vkCmdBlitImage(commandBuffer,
                        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &blit,
                        VK_FILTER_LINEAR);

                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);
                }

                if (mipWidth > 1)
                {
                    mipWidth /= 2;
                }

                if (mipHeight > 1)
                {
                    mipHeight /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1; // 각 레이어에 대해 개별적으로 전환
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            for (uint32_t i = 0; i < 6; i++) {
                barrier.subresourceRange.baseArrayLayer = i;
                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
            }

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

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
            _VK_CHECK_RESULT_(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));  // 커맨드 버퍼를 할당합니다.

            VkCommandBufferBeginInfo beginInfo{};
            // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO : 명령 버퍼의 시작 정보를 설정합니다.
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 커맨드 버퍼를 한 번만 사용하려는 경우 사용합니다.
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            // 커맨드 버퍼를 시작합니다.
            _VK_CHECK_RESULT_(vkBeginCommandBuffer(commandBuffer, &beginInfo));

            return commandBuffer;

        }
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
        {
            // 커맨드 버퍼를 종료합니다.
            _VK_CHECK_RESULT_(vkEndCommandBuffer(commandBuffer));

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1; // 커맨드 버퍼 개수를 설정합니다.
            submitInfo.pCommandBuffers = &commandBuffer; // 커맨드 버퍼를 설정합니다.

            // 큐에 커맨드 버퍼를 제출합니다.
            _VK_CHECK_RESULT_(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

            // 큐가 모든 작업을 완료할 때까지 대기합니다.
            _VK_CHECK_RESULT_(vkQueueWaitIdle(graphicsQueue));

            // 커맨드 버퍼를 해제합니다.
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            cUint32_t levelCount,
            cUint32_t layerCount)
        {
            // 단일 시간 명령 버퍼를 시작합니다.
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // 이미지 메모리 배리어 구조체를 초기화하여 레이아웃 전환 및 접근 권한 변경을 정의합니다.
            // 기본적으로 색상 정보를 대상으로 하지만, 이후 조건에 따라 수정됩니다.
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // 배리어의 구조체 타입 설정
            barrier.oldLayout = oldLayout;                          // 전환 전 이미지 레이아웃
            barrier.newLayout = newLayout;                          // 전환할 이미지 레이아웃
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시
            barrier.image = image; // 전환할 이미지 핸들 설정
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // 첫 번째 미프맵 레벨부터 시작
            barrier.subresourceRange.levelCount = levelCount;// 적용할 미프맵의 수
            barrier.subresourceRange.baseArrayLayer = 0;     // 첫 번째 배열 레이어부터 시작
            barrier.subresourceRange.layerCount = layerCount;         // 배열 내 레이어 개수

            VkPipelineStageFlags sourceStage = 0;      // 전환 전 파이프라인 스테이지
            VkPipelineStageFlags destinationStage = 0; // 전환 후 파이프라인 스테이지

            // 이미지 레이아웃 전환 시, 적절한 액세스 마스크 및 파이프라인 스테이지를 설정합니다.

            switch (oldLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                // or 깊이/스텐실 이미지 초기화 경우로, 이전 레이아웃이 사용 불가능한 상태에서 시작함
                barrier.srcAccessMask = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

            switch (newLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // 전송 완료 후 색상 첨부 최적화 레이아웃으로 전환
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 쓰기 작업을 위한 쓰기 액세스 허용 설정
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

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
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            }
            else {
                // 지원되지 않는 레이아웃 전환 요청시 예외 발생
                // 1, 모든 파이프라인 스테이지를 사용하여 전환

                sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

                _PRINT_TO_CONSOLE_("All pass pipeline stage!\n");
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

        void updateimageLayoutcmd(VkCommandBuffer cmdbuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, cUint32_t levelCount, cUint32_t layerCount)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // 배리어의 구조체 타입 설정
            barrier.oldLayout = oldLayout;                          // 전환 전 이미지 레이아웃
            barrier.newLayout = newLayout;                          // 전환할 이미지 레이아웃
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시
            barrier.image = image; // 전환할 이미지 핸들 설정
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // 첫 번째 미프맵 레벨부터 시작
            barrier.subresourceRange.levelCount = levelCount;// 적용할 미프맵의 수
            barrier.subresourceRange.baseArrayLayer = 0;     // 첫 번째 배열 레이어부터 시작
            barrier.subresourceRange.layerCount = layerCount;         // 배열 내 레이어 개수

            VkPipelineStageFlags sourceStage = 0;      // 전환 전 파이프라인 스테이지
            VkPipelineStageFlags destinationStage = 0; // 전환 후 파이프라인 스테이지

            // 이미지 레이아웃 전환 시, 적절한 액세스 마스크 및 파이프라인 스테이지를 설정합니다.

            barrier.srcAccessMask = getFromOldLayoutToVkAccessFlags(oldLayout);
            barrier.dstAccessMask = getFromNewLayoutToVkAccessFlags(newLayout);

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
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            }
            else {
                // 지원되지 않는 레이아웃 전환 요청시 예외 발생
                // 1, 모든 파이프라인 스테이지를 사용하여 전환

                sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

                _PRINT_TO_CONSOLE_("All pass pipeline stage!\n");
            }

            // 파이프라인 배리어를 추가하여 레이아웃 전환 명령을 기록합니다.
            vkCmdPipelineBarrier(
                cmdbuffer,
                sourceStage, destinationStage, // 전환 전/후 스테이지
                0,                // 배리어 플래그 (사용하지 않음)
                0, nullptr,     // 메모리 배리어 없이
                0, nullptr,     // 버퍼 배리어 없이
                1, &barrier     // 하나의 이미지 배리어 사용
            );
        }

    }
}