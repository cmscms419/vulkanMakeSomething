#include "helperCreate.h"
#include "helper.h"

using namespace vkengine::Log;

namespace vkengine {
    namespace helper {

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
            allocInfo.memoryTypeIndex = vkengine::helper::findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);

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
            allocInfo.memoryTypeIndex = vkengine::helper::findMemoryType(VKphysicalDevice, memRequirements.memoryTypeBits, properties);
            _VK_CHECK_RESULT_(vkAllocateMemory(VKdevice, &allocInfo, nullptr, &imageMemory));
            _VK_CHECK_RESULT_(vkBindImageMemory(VKdevice, image, imageMemory, 0));
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
            allocInfo.memoryTypeIndex = vkengine::helper::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);     // 요구사항에 맞는 메모리 타입 인덱스를 찾아서 지정한다.

            _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));  // 위 정보를 바탕으로 메모리를 할당하고 성공 여부를 검사한다.
            _VK_CHECK_RESULT_(vkBindBufferMemory(device, buffer, bufferMemory, 0));           // 할당된 메모리를 버퍼와 바인딩하여 GPU에서 사용할 수 있게 한다.
        }

        void createBuffer2(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize* allocatedSize, VkDeviceSize* alignment)
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

            *allocatedSize = memRequirements.size; // 실제 할당된 크기
            *alignment = memRequirements.alignment; // 정렬 방식

            PRINT_TO_LOGGER("allocatedSize : %d, alignment : %d \n", *allocatedSize, *alignment);

            // 메모리 할당 정보를 담은 구조체를 초기화한다.
            VkMemoryAllocateInfo allocInfo{};

            // 만약, 버퍼를 포인트 기반으로 사용한다면, 해당 if문을 타고 사용할 수 있다.
            VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
            if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
                allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
                allocInfo.pNext = &allocFlagsInfo;
            }

            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;                                                   // 구조체 타입을 지정한다.    
            allocInfo.allocationSize = memRequirements.size;                                                            // 버퍼를 위한 메모리 크기를 설정한다.
            allocInfo.memoryTypeIndex = vkengine::helper::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);     // 요구사항에 맞는 메모리 타입 인덱스를 찾아서 지정한다.

            _VK_CHECK_RESULT_(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));  // 위 정보를 바탕으로 메모리를 할당하고 성공 여부를 검사한다.
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

        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
        {
            VkCommandPoolCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            info.pNext = nullptr;
            info.queueFamilyIndex = queueFamilyIndex;
            info.flags = flags;
            return info;
        }

        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
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

        SpvReflectShaderModule createSpvReflectModule(const std::vector<cChar>& code)
        {
            SpvReflectShaderModule reflectShaderModule;
            SpvReflectResult reflectResult = spvReflectCreateShaderModule(
                code.size(),
                reinterpret_cast<const cUint32_t*>(code.data()),
                &reflectShaderModule);
            if (reflectResult != SPV_REFLECT_RESULT_SUCCESS) {
                EXIT_TO_LOGGER("Failed to create SPIR-V reflection module: %d", reflectResult);
            }

            if (reflectShaderModule._internal == nullptr) {
                EXIT_TO_LOGGER("SPIR-V reflection module internal structure is null.");
            }

            return reflectShaderModule;
        }

        VkShaderModule createShaderModule(VkDevice device, const std::vector<cChar>& code)
        {
            VkShaderModuleCreateInfo createInfo{};

            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule{ VK_NULL_HANDLE };

            _VK_CHECK_RESULT_(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

            return shaderModule;
        }


    }
}