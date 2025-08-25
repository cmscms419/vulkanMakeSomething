#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include <set>
#include <fstream>
#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKbuffer.h"
#include "VKdevice.h"
#include "ktx.h"

// https://github.com/SaschaWillems/Vulkan���� �����ؼ� �Լ� ����

namespace vkengine {
    struct VKdeviceHandler;

    namespace helper {

        VkAccessFlags getFromOldLayoutToVkAccessFlags(VkImageLayout format);

        VkAccessFlags getFromNewLayoutToVkAccessFlags(VkImageLayout format);

        // ������ �о���� �Լ�
        std::vector<cChar> readFile(const std::string& filename);

        // ���� ����̽��� �䱸 ������ �����ϴ��� Ȯ���ϴ� �Լ�
        cBool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices);

        // �־��� ���� ��ġ���� ť �йи� �Ӽ��� ã�� �Լ�
        // PROB : ť �йи��� �������� ��쿡 �ʿ��� ó���� �ִ� �йи��� ���� ã�� ���, �� �йи��� �ε����� ��ȯ��
        // TODO ; ť �йи��� �������� ��쿡 ���� ó���� �ʿ���
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface);

        inline VkViewport createViewport(cFloat x, cFloat y, cFloat width, cFloat height, cFloat minDepth = 0.0f, cFloat maxDepth = 1.0f)
        {
            VkViewport viewport{};
            viewport.x = x;
            viewport.y = y;
            viewport.width = width;
            viewport.height = height;
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;

            return viewport;
        }
        
        inline VkRect2D createScissor(cInt32_t offsetX, cInt32_t offsetY, cUint32_t width, cUint32_t height)
        {
            VkRect2D scissor{};
            scissor.offset.x = offsetX;
            scissor.offset.y = offsetY;
            scissor.extent.width = width;
            scissor.extent.height = height;

            return scissor;
        }

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

        void createImage2(
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
            uint32_t arrayLayer,
            VkImageCreateFlagBits flag);


        // ���۸� �����ϴ� �Լ�
        void copyBuffer(
            VkDevice VKdevice,
            VkCommandPool VKcommandPool,
            VkQueue graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        void copyBuffer2(
            vkengine::VKdeviceHandler& VKdevice,
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

        void copyBufferToImageKTX(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height,
            uint32_t mipmapLevels,
            ktxTexture* textureKTX);

        // ���� ����̽��� Ȯ�� ����� �����ϴ��� Ȯ���ϴ� �Լ�
        // ���� ���̾� ���� ���θ� Ȯ���ϴ� �Լ�
        // Ȯ�� ����� �����ϰ� �ʿ��� ��� Ȯ�� ����� ���ԵǾ� �ִ��� Ȯ��
        cBool checkDeviceExtensionSupport(VkPhysicalDevice device);

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
            cSize imageCount = 1);

        VkImageView createArrayImageView(
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels,
            cSize imageCount);

        VkImageView createCubeImageView(
            VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

        // ���ٽ� ������Ʈ�� ������ �ִ��� Ȯ���ϴ� �Լ�
        inline cBool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        // Mipmaps�� �����ϴ� �Լ�
        // �̹����� �����ϰ� �̹��� ���̾ƿ��� ������ ���� �̹����� ���� -> ���� �̹����� ���� mipmap ����
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

        // CubeMap�� ���� mipmap�� �����ϴ� �Լ�
        void generateMipmapsCubeMap(
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

        /**
            * @brief �̹����� ���̾ƿ��� ��ȯ�ϴ� �Լ�
            *
            * �� �Լ��� ������ �̹����� ���� �ʿ信 ���� �̹��� �޸� �踮� �����ϰ�,
            * ���� ���̾ƿ�(oldLayout)���� ���ο� ���̾ƿ�(newLayout)���� �̹����� �����մϴ�.
            * �̹��� ��ȯ�� ���� �ð� ��� ���۸� ����Ͽ� ����Ǹ�, ��ȯ �۾� �� ���� ��� ���۴�
            * ����Ǿ� �Ϸ�� ������ ���˴ϴ�.
            *
            * ->���� oldLayout�� ���� imageLayout�� newLayout���� �̹����� ��� �� �� �̿��ϴ� �Լ�
            *
            * @remark �����Ǵ� ��ȯ�� �Ʒ��� 3���� ����Դϴ�:
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
            * @brief �̹��� ���̾ƿ� ��ȯ ����� Ŀ�ǵ� ���ۿ� ����ϴ� �Լ�
            *
            * �� �Լ��� �־��� VkCommandBuffer�� �̹��� ���̾ƿ� ��ȯ�� ���� VkImageMemoryBarrier�� ����մϴ�.
            * oldLayout���� newLayout������ ��ȯ�� ���� ������ ���� ����ũ�� ���������� ���������� �����մϴ�.
            * mipmap level�� array layer ������ ������ �� �ֽ��ϴ�.
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

        // stencilComponent�� ������ �ִ��� Ȯ���ϴ� �Լ�
        cBool hasStencilComponent(VkFormat format);

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

        // VkWriteDescriptorSet ����ü�� �����ϴ� �Լ���
        
        // Buffer�� ����ϴ� ���
        inline VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            uint32_t descriptorCount = 1)
        {
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = dstSet;
            writeDescriptorSet.descriptorType = type;
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.pBufferInfo = bufferInfo;
            writeDescriptorSet.descriptorCount = descriptorCount;
            return writeDescriptorSet;
        }

        // Image�� ����ϴ� ���
        inline VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            uint32_t binding,
            VkDescriptorImageInfo* imageInfo,
            uint32_t descriptorCount = 1)
        {
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = dstSet;
            writeDescriptorSet.descriptorType = type;
            writeDescriptorSet.dstBinding = binding;
            writeDescriptorSet.pImageInfo = imageInfo;
            writeDescriptorSet.descriptorCount = descriptorCount;
            return writeDescriptorSet;
        }

        inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            uint32_t maxSets)
        {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            descriptorPoolInfo.pPoolSizes = poolSizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            return descriptorPoolInfo;
        }

        inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            uint32_t binding,
            uint32_t descriptorCount = 1)
        {
            VkDescriptorSetLayoutBinding setLayoutBinding{};
            setLayoutBinding.descriptorType = type;
            setLayoutBinding.stageFlags = stageFlags;
            setLayoutBinding.binding = binding;
            setLayoutBinding.descriptorCount = descriptorCount;
            return setLayoutBinding;
        }

        inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
            const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
            descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.pBindings = bindings.data();
            descriptorSetLayoutCreateInfo.bindingCount = static_cast<cUint32_t>(bindings.size());
            return descriptorSetLayoutCreateInfo;
        }

        inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo2(
            const VkDescriptorSetLayoutBinding& bindings,
            cUint32_t bindingCount = 1
            )
        {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
            descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.pBindings = &bindings;
            descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
            return descriptorSetLayoutCreateInfo;
        }

        inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
            const VkDescriptorPool& descriptorPool,
            const VkDescriptorSetLayout& pSetLayouts,
            cUint32_t descriptorSetCount = 1)
        {
            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = descriptorPool;
            descriptorSetAllocateInfo.pSetLayouts = &pSetLayouts;
            descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;

            return descriptorSetAllocateInfo;
        }

        inline VkDescriptorPoolSize descriptorPoolSize(
            VkDescriptorType type,
            cUint32_t descriptorCount)
        {
            VkDescriptorPoolSize descriptorPoolSize{};
            descriptorPoolSize.type = type;
            descriptorPoolSize.descriptorCount = descriptorCount;
            return descriptorPoolSize;
        }

        // VkPipelineShaderStageCreateInfo ����
        // �� �Լ��� ���̴� ���� ��Ʈ�� ����Ʈ�� ����Ͽ� ���������� ���̴� �������� ������ �����մϴ�.
        inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(
            VkShaderStageFlagBits stage,
            VkShaderModule shaderModule,
            cChar* entryPoint = "main")
        {
            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = stage;
            shaderStageInfo.module = shaderModule;
            shaderStageInfo.pName = entryPoint;
            return shaderStageInfo;
        }

        // VkPipelineVertexInputStateCreateInfo ����
        // �� �Լ��� ���ؽ� �Է� ���ε� �� �Ӽ� ������ ����Ͽ� ���������� ���ؽ� �Է� ���� ������ �����մϴ�.
        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
            const VkVertexInputBindingDescription& bindingDescriptions,
            const VkVertexInputAttributeDescription& attributeDescriptions,
            const cUint32_t attributeDescriptionsSize,
            const cUint32_t bindingDescriptionCount
            )
        {
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptionCount;
            vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionsSize;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions;
            vertexInputInfo.pVertexAttributeDescriptions = &attributeDescriptions;

            return vertexInputInfo;
        }

        // VkPipelineInputAssemblyStateCreateInfo ����
        inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
            VkPrimitiveTopology topology,
            VkPipelineInputAssemblyStateCreateFlags flags,
            VkBool32 primitiveRestartEnable)
        {
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = topology;
            pipelineInputAssemblyStateCreateInfo.flags = flags;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
            return pipelineInputAssemblyStateCreateInfo;
        }

        // ����Ʈ ����
        inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
            const VkViewport& viewport,
            const VkRect2D& scissor,
            cUint32_t viewportCount = 1,
            cUint32_t scissorCount = 1,
            VkPipelineViewportStateCreateFlags flags = 0)
        {
            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = viewportCount;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = scissorCount;
            viewportState.pScissors = &scissor;
            viewportState.flags = flags;
            return viewportState;
        }

        // ������ȭ ����
        inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
            VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
            VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE,
            VkBool32 depthClampEnable = VK_FALSE,
            VkBool32 rasterizerDiscardEnable = VK_FALSE,
            float lineWidth = 1.0f,
            VkBool32 depthBiasEnable = VK_FALSE,
            float depthBiasConstantFactor = 0.0f,
            float depthBiasClamp = 0.0f,
            float depthBiasSlopeFactor = 0.0f
            )
        {
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = depthClampEnable;
            rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
            rasterizer.polygonMode = polygonMode;
            rasterizer.lineWidth = lineWidth;
            rasterizer.cullMode = cullMode;
            rasterizer.frontFace = frontFace;
            rasterizer.depthBiasEnable = VK_FALSE;                    // ���� ���̾ ��Ȱ��ȭ
            rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;              // ���� ���̾ ��� ��Ҹ� 0.0f�� ����
            rasterizer.depthBiasClamp = depthBiasClamp;                       // ���� ���̾ Ŭ������ 0.0f�� ����
            rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;                 // ���� ���̾ ������ ��Ҹ� 0.0f�� ����
            return rasterizer;
        }

        // ���� ���ø� ����
        inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
            VkSampleCountFlagBits rasterizationSamples,
            VkPipelineMultisampleStateCreateFlags flags = 0)
        {
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
            pipelineMultisampleStateCreateInfo.flags = flags;
            return pipelineMultisampleStateCreateInfo;
        }

        // ���� ���ٽ� �׽�Ʈ ����
        inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
            VkBool32 depthTestEnable,
            VkBool32 depthWriteEnable,
            VkCompareOp depthCompareOp)
        {
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
            pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
            pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
            pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
            pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
            return pipelineDepthStencilStateCreateInfo;
        }

        inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
            VkColorComponentFlags colorWriteMask,
            VkBool32 blendEnable,
            VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD,
            VkBlendOp colorBlendOp = VK_BLEND_OP_ADD,
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO
            )
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = colorWriteMask;;                // �÷� ���� ������ ����
            colorBlendAttachment.blendEnable = blendEnable;
            colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor;     // �ҽ� �÷� ���� ���͸� ����
            colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;    // ��� �÷� ���� ���͸� ����
            colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;     // �ҽ� ���� ���� ���͸� ����
            colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;    // ��� ���� ���� ���͸� ����
            colorBlendAttachment.alphaBlendOp = alphaBlendOp;                // ���� ���� ������ ����
            colorBlendAttachment.colorBlendOp = colorBlendOp;                // �÷� ���� ������ ����

            return colorBlendAttachment;
        }

        // �÷� ���� ���� ���� ���� ����ü
        inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
            uint32_t attachmentCount,
            const VkPipelineColorBlendAttachmentState* pAttachments)
        {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
            pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
            return pipelineColorBlendStateCreateInfo;
        }

        // ���̳��� ���� ���� -> ������ȭ ���¸� �������� ������ �� �ֽ��ϴ�.
        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& dynamicStates)
        {
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();
            return dynamicState;
        }

        inline VkPushConstantRange pushConstantRange(
            VkShaderStageFlags stageFlags,
            uint32_t size,
            uint32_t offset)
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags;
            pushConstantRange.offset = offset;
            pushConstantRange.size = size;
            return pushConstantRange;
        }

        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            uint32_t setLayoutCount = 1)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
            pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
            return pipelineLayoutCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
            VkPipelineLayout layout,
            VkRenderPass renderPass,
            VkPipelineCreateFlags flags = 0)
        {
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.layout = layout;
            pipelineCreateInfo.renderPass = renderPass;
            pipelineCreateInfo.flags = flags;
            pipelineCreateInfo.basePipelineIndex = -1;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            return pipelineCreateInfo;
        }

        bool fileExists(const std::string& filename);
        uint32_t alignedSize(uint32_t value, uint32_t alignment);
        size_t alignedSize(size_t value, size_t alignment);
        VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment);

        // �׷��� ���������� ����
    }

}


#endif // !INCLUDE_SOURCE_HELPER_H_