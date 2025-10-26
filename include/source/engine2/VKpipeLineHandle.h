#ifndef VK_PIPELINEhANDLE_H_
#define VK_PIPELINEhANDLE_H_

#include "common.h"
#include "struct.h"
#include "log.h"

#include "VKContext.h"
#include "VKShaderManager.h"

namespace vkengine {
    class VKPipeLineHandle {
    public:

        VKPipeLineHandle(VkContext& context, VKShaderManager& shaderManager) : ctx(context), shaderManager(shaderManager)
        {

        }

        VKPipeLineHandle(VkContext& ctx, VKShaderManager& shaderManager, cString Name,
            VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
            : ctx(ctx), shaderManager(shaderManager), name(Name)
        {
            createCommon();
            createGraphicsPipeline(outColorFormat, depthFormat, msaaSamples);
        }

        VKPipeLineHandle(VKPipeLineHandle&& other) noexcept
            : ctx(other.ctx), shaderManager(other.shaderManager),
            pipelineLayout(other.pipelineLayout), pipeline(other.pipeline), 
            name(std::move(other.name))
        {
            other.pipelineLayout = VK_NULL_HANDLE;
            other.pipeline = VK_NULL_HANDLE;
        }

        VKPipeLineHandle& operator=(VKPipeLineHandle&& other) noexcept
        {
            if (this != &other)
            {
                cleanup();
                pipelineLayout = other.pipelineLayout;
                pipeline = other.pipeline;
                name = std::move(other.name);
                other.pipelineLayout = VK_NULL_HANDLE;
                other.pipeline = VK_NULL_HANDLE;
            }
            return *this;
        }

        VKPipeLineHandle(const VKPipeLineHandle&) = delete;
        VKPipeLineHandle& operator=(const VKPipeLineHandle&) = delete;

        ~VKPipeLineHandle()
        {
            cleanup();
        }

        void cleanup();

        // 공통으로 사용되는 파이프라인 설정을 여기에 추가할 수 있습니다.
        void createCommon();

        void createGraphicsPipeline(VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createSquarePipeline(
            VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);


        VkPipelineLayout* getPipelineLayout() { return &pipelineLayout; }
        VkPipeline* getPipeline() { return &pipeline; }

    private:
        VkContext& ctx;
        VKShaderManager& shaderManager;

        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        VkPipeline pipeline{ VK_NULL_HANDLE };

        cString name{};

    };
}

#endif // !VK_PIPELINEhANDLE_H_
