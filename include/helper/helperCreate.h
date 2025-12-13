#ifndef INCLUDE_HELPER_CREATE_H_
#define INCLUDE_HELPER_CREATE_H_

#include <set>
#include <fstream>

#include "common.h"
#include "struct.h"
#include "log.h"

// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine {

    namespace helper {

        // VkPipelineShaderStageCreateInfo 생성
        // 이 함수는 셰이더 모듈과 엔트리 포인트를 사용하여 파이프라인 셰이더 스테이지 정보를 생성합니다.
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

        // VkPipelineVertexInputStateCreateInfo 생성
        // 이 함수는 버텍스 입력 바인딩 및 속성 설명을 사용하여 파이프라인 버텍스 입력 상태 정보를 생성합니다.
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

        // VkPipelineInputAssemblyStateCreateInfo 생성
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

        // 뷰포트 설정
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

        // 래스터화 설정
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
            rasterizer.pNext = nullptr;
            rasterizer.flags = 0;
            rasterizer.depthClampEnable = depthClampEnable;
            rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
            rasterizer.polygonMode = polygonMode;
            rasterizer.lineWidth = lineWidth;
            rasterizer.cullMode = cullMode;
            rasterizer.frontFace = frontFace;
            rasterizer.depthBiasEnable = VK_FALSE;                    // 깊이 바이어스 비활성화
            rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;              // 깊이 바이어스 상수 요소를 0.0f로 설정
            rasterizer.depthBiasClamp = depthBiasClamp;                       // 깊이 바이어스 클램프를 0.0f로 설정
            rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;                 // 깊이 바이어스 슬로프 요소를 0.0f로 설정
            return rasterizer;
        }

        // 다중 샘플링 설정
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

        // 깊이 스텐실 테스트 설정
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
            colorBlendAttachment.colorWriteMask = colorWriteMask;;                // 컬러 블렌딩 연산을 설정
            colorBlendAttachment.blendEnable = blendEnable;
            colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor;     // 소스 컬러 블렌딩 팩터를 설정
            colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;    // 대상 컬러 블렌딩 팩터를 설정
            colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;     // 소스 알파 블렌딩 팩터를 설정
            colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;    // 대상 알파 블렌딩 팩터를 설정
            colorBlendAttachment.alphaBlendOp = alphaBlendOp;                // 알파 블렌딩 연산을 설정
            colorBlendAttachment.colorBlendOp = colorBlendOp;                // 컬러 블렌딩 연산을 설정

            return colorBlendAttachment;
        }

        // 컬러 블렌딩 상태 생성 정보 구조체
        inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
            cUint32_t attachmentCount,
            const VkPipelineColorBlendAttachmentState* pAttachments)
        {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
            pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
            return pipelineColorBlendStateCreateInfo;
        }

        // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& dynamicStates)
        {
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<cUint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();
            return dynamicState;
        }

        inline VkPushConstantRange pushConstantRange(
            VkShaderStageFlags stageFlags,
            cUint32_t size,
            cUint32_t offset)
        {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags;
            pushConstantRange.offset = offset;
            pushConstantRange.size = size;
            return pushConstantRange;
        }

        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            const VkDescriptorSetLayout* pSetLayouts,
            cUint32_t setLayoutCount = 1)
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

        inline VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo(
            std::vector<VkFormat> colorAttachmentFormats,
            VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED,
            VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED)
        {
            static thread_local std::vector<VkFormat> formats; // thread-safe한 정적 저장소
            formats = std::move(colorAttachmentFormats);

            VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
            pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            pipelineRenderingCreateInfo.colorAttachmentCount = static_cast<cUint32_t>(colorAttachmentFormats.size());
            pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
            pipelineRenderingCreateInfo.depthAttachmentFormat = depthAttachmentFormat;
            pipelineRenderingCreateInfo.stencilAttachmentFormat = stencilAttachmentFormat;
            return pipelineRenderingCreateInfo;
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

        // VkWriteDescriptorSet 구조체를 생성하는 함수들

        // Buffer를 사용하는 경우
        inline VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            cUint32_t binding,
            VkDescriptorBufferInfo* bufferInfo,
            cUint32_t descriptorCount = 1)
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

        // Image를 사용하는 경우
        inline VkWriteDescriptorSet writeDescriptorSet(
            VkDescriptorSet dstSet,
            VkDescriptorType type,
            cUint32_t binding,
            VkDescriptorImageInfo* imageInfo,
            cUint32_t descriptorCount = 1)
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
            cUint32_t maxSets)
        {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<cUint32_t>(poolSizes.size());
            descriptorPoolInfo.pPoolSizes = poolSizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            return descriptorPoolInfo;
        }

        inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
            VkDescriptorType type,
            VkShaderStageFlags stageFlags,
            cUint32_t binding,
            cUint32_t descriptorCount = 1)
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

        SpvReflectShaderModule createSpvReflectModule(const std::vector<cChar>& code);

        VkShaderModule createShaderModule(VkDevice device, const std::vector<cChar>& code);

        void createImage(
            VkDevice VKdevice,
            VkPhysicalDevice VKphysicalDevice,
            cUint32_t width,
            cUint32_t height,
            cUint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory,
            cUint32_t arrayLayer = 1);

        void createImage2(
            VkDevice VKdevice,
            VkPhysicalDevice VKphysicalDevice,
            cUint32_t width,
            cUint32_t height,
            cUint32_t mipLevels,
            VkSampleCountFlagBits numSamples,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory,
            cUint32_t arrayLayer,
            VkImageCreateFlagBits flag);

        // buffer를 생성하는 함수
        void createBuffer(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);

        // buffer를 생성하는 함수 2
        void createBuffer2(
            VkDevice device,
            VkPhysicalDevice physicalDevice,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory,
            VkDeviceSize* allocatedSize,
            VkDeviceSize* alignment
        );

        VkImageView createImageView(
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            cUint32_t mipLevels,
            cSize imageCount = 1);

        VkImageView createArrayImageView(
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspectFlags,
            cUint32_t mipLevels,
            cSize imageCount);

        VkImageView createCubeImageView(
            VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, cUint32_t mipLevels);


        // commandPood에 대한 information을 생성하는 함수
        VkCommandPoolCreateInfo commandPoolCreateInfo(cUint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

        // commandBuffer에 대한 information을 생성하는 함수
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, cUint32_t count, VkCommandBufferLevel level);

        // Fencee에 대한 information을 생성하는 함수
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

        // Semaphore에 대한 information을 생성하는 함수
        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

    }

}


#endif // !INCLUDE_HELPER_CREATE_H_