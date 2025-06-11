#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include <set>
#include <fstream>

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKbuffer.h"
#include "VKdevice.h"

// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine {
    struct VKDevice_;

    namespace helper {

        // 파일을 읽어오는 함수
        std::vector<cChar> readFile(const std::string& filename);

        // 물리 디바이스가 요구 사항을 충족하는지 확인하는 함수
        cBool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices);

        // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
        // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
        // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface);

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


        // 버퍼를 복사하는 함수
        void copyBuffer(
            VkDevice VKdevice,
            VkCommandPool VKcommandPool,
            VkQueue graphicsVKQueue,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        void copyBuffer2(
            vkengine::VKDevice_& VKdevice,
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize size);

        // buffer를 생성하는 함수
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

        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
        // 검증 레이어 지원 여부를 확인하는 함수
        // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
        cBool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // 물리 디바이스의 모든 확장기능을 가져오는 함수
        void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<std::string>* temp);

        // 물리 디바이스의 적합성을 평가하는 함수
        int rateDeviceSuitability(VkPhysicalDevice device);

        // 스왑 체인 지원 정보를 가져오는 함수
        // 스왑 체인 지원 정보를 저장할 구조체를 초기화
        // 물리 장치에서 서피스의 기능을 가져옴
        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface);

        // 깊이 형식을 찾는 함수
        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

        // Format을 지원하는지 확인하는 함수
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

        // 스텐실 컴포넌트를 가지고 있는지 확인하는 함수
        inline cBool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        // Mipmaps을 생성하는 함수
        // 이미지를 생성하고 이미지 레이아웃을 변경한 다음 이미지를 복사 -> 단일 이미지에 대한 mipmap 생성
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

        // CubeMap에 대한 mipmap을 생성하는 함수
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

        // 최대 사용 가능한 샘플링 수를 반환하는 함수

        //VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer 나중에 추가
        // flushSetupCommands 나중에 추가

        // pyhsicalDeviceProperties를 가져오는 함수
        inline void getPyhsicalDeviceProperties(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            _PRINT_TO_CONSOLE_("Device Name: %s\n", deviceProperties.deviceName);
            _PRINT_TO_CONSOLE_("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        }

        // commandPood에 대한 information을 생성하는 함수
        VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

        // commandBuffer에 대한 information을 생성하는 함수
        VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Fencee에 대한 information을 생성하는 함수
        VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

        // Semaphore에 대한 information을 생성하는 함수
        VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

        // https://steel-gourd-618.notion.site/Images-19618a41dc6f80b89bc1d1575bcf3d04 참고
        //  시작하려는 명령버퍼를 생성하는 함수
        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

        // 명령버퍼를 종료하는 함수
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);

        // 이미지 레이아웃을 전환하는 함수
        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels);

        // 이미지배열 레이아웃을 전환하는 함수
        void transitionImageLayout2(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels,
            cSize layerCount);

        void transitionImageLayout3(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels,
            cSize layerCount,
            VkImageAspectFlags srcStageMask,
            VkImageAspectFlags dstStageMask);

        void setImageLayout(
            VkCommandBuffer cmdbuffer,
            VkImage image,
            VkFormat format,
            VkImageLayout oldImageLayout,
            VkImageLayout newImageLayout,
            VkImageSubresourceRange subresourceRange
        );

        // stencilComponent를 가지고 있는지 확인하는 함수
        cBool hasStencilComponent(VkFormat format);

        // 특정 데이터를 VkDeviceMemory 객체로 복사하는 함수
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

        // VkWriteDescriptorSet 구조체를 생성하는 함수들
        
        // Buffer를 사용하는 경우
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

        // Image를 사용하는 경우
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
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VkBool32 primitiveRestartEnable = VK_FALSE)
        {
            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = topology;
            inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
            return inputAssembly;
        }

        // 뷰포트 설정
        inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
            const VkViewport& viewport,
            const VkRect2D& scissor,
            cUint32_t viewportCount = 1,
            cUint32_t scissorCount = 1)
        {
            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = viewportCount;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = scissorCount;
            viewportState.pScissors = &scissor;
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
            VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            VkBool32 sampleShadingEnable = VK_FALSE,
            float minSampleShading = 1.0f,
            const VkSampleMask* pSampleMask = nullptr,
            VkBool32 alphaToCoverageEnable = VK_FALSE,
            VkBool32 alphaToOneEnable = VK_FALSE)
        {
            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = sampleShadingEnable;
            multisampling.rasterizationSamples = rasterizationSamples;
            multisampling.minSampleShading = minSampleShading;
            multisampling.pSampleMask = pSampleMask;
            multisampling.alphaToCoverageEnable = alphaToCoverageEnable;
            multisampling.alphaToOneEnable = alphaToOneEnable;
            return multisampling;
        }

        // 깊이 스텐실 테스트 설정
        inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
            VkBool32 depthTestEnable = VK_TRUE,
            VkBool32 depthWriteEnable = VK_FALSE,
            VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS,
            VkBool32 depthBoundsTestEnable = VK_FALSE,
            VkBool32 stencilTestEnable = VK_FALSE,
            const VkStencilOpState& front = {},
            const VkStencilOpState& back = {},
            float minDepthBounds = 0.0f,
            float maxDepthBounds = 1.0f)
        {
            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = depthTestEnable;
            depthStencil.depthWriteEnable = depthWriteEnable;
            depthStencil.depthCompareOp = depthCompareOp;
            depthStencil.depthBoundsTestEnable = depthBoundsTestEnable;
            depthStencil.stencilTestEnable = stencilTestEnable;
            depthStencil.front = front;
            depthStencil.back = back;
            depthStencil.minDepthBounds = minDepthBounds;
            depthStencil.maxDepthBounds = maxDepthBounds;
            return depthStencil;
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
            const VkPipelineColorBlendAttachmentState& attachment,
            VkBool32 logicOpEnable = VK_FALSE,
            VkLogicOp logicOp = VK_LOGIC_OP_COPY,
            const cFloat* blendConstants = nullptr// 4개의 블렌드 상수 (R, G, B, A)
        )
        {
            VkPipelineColorBlendStateCreateInfo colorBlending{};

            if (!blendConstants)
            {
                static float defaultBlendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                blendConstants = defaultBlendConstants;
                _PRINT_TO_CONSOLE_("Using default blend constants: [0.0f, 0.0f, 0.0f, 0.0f]\n");
            }

            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = logicOpEnable;
            colorBlending.logicOp = logicOp;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &attachment;
            colorBlending.blendConstants[0] = blendConstants[0];
            colorBlending.blendConstants[1] = blendConstants[1];
            colorBlending.blendConstants[2] = blendConstants[2];
            colorBlending.blendConstants[3] = blendConstants[3];

            return colorBlending;
        }

        // 다이나믹 상태 설정 -> 레스터화 상태를 동적으로 변경할 수 있습니다.
        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& dynamicStates)
        {
            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();
            return dynamicState;
        }


        // 그래픽 파이프라인 생성
    }

}


#endif // !INCLUDE_SOURCE_HELPER_H_