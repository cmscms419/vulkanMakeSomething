#ifndef VK_PIPELIN_INCLUDE_H_
#define VK_PIPELIN_INCLUDE_H_

#include "common.h"
#include "struct.h"
#include "log.h"
#include <optional>

#include "VKContext.h"
#include "VKShaderManager.h"

namespace vkengine {
    class PipeLineHandle {
    public:

        PipeLineHandle(VKcontext& context, VKShaderManager& shaderManager) : ctx(context), shaderManager(shaderManager)
        {

        }

        PipeLineHandle(VKcontext& ctx, VKShaderManager& shaderManager, cString Name,
            VkFormat outColorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamples)
            : ctx(ctx), shaderManager(shaderManager), name(Name)
        {
            createByName(name, outColorFormat, depthFormat, msaaSamples);
        }

        PipeLineHandle(PipeLineHandle&& other) noexcept
            : ctx(other.ctx), shaderManager(other.shaderManager),
            pipelineLayout(other.pipelineLayout), pipeline(other.pipeline), 
            name(std::move(other.name))
        {
            other.pipelineLayout = VK_NULL_HANDLE;
            other.pipeline = VK_NULL_HANDLE;
        }

        PipeLineHandle& operator=(PipeLineHandle&& other) noexcept
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

        PipeLineHandle(const PipeLineHandle&) = delete;
        PipeLineHandle& operator=(const PipeLineHandle&) = delete;

        ~PipeLineHandle()
        {
            cleanup();
        }

        void cleanup();

        // 공통으로 사용되는 파이프라인 설정을 여기에 추가할 수 있습니다.
        void createCommon();


        void createByName(cString name,
            std::optional<VkFormat> outColorFormat = VK_FORMAT_UNDEFINED,
            std::optional<VkFormat> depthFormat = VK_FORMAT_UNDEFINED,
            std::optional<VkSampleCountFlagBits> msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createSquarePipeline(
            VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);

        void createSkyboxPipeline(
            VkFormat outColorFormat, 
            VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples);

        void createGuiPipeline(VkFormat outColorFormat = VK_FORMAT_UNDEFINED);
        void createPostProcessingPipeLine(
            VkFormat outColorFormat = VK_FORMAT_UNDEFINED,
            VkFormat depthFormat = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM);


        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
        VkPipeline getPipeline() { return pipeline; }

    private:
        VKcontext& ctx;
        VKShaderManager& shaderManager;

        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
        VkPipeline pipeline{ VK_NULL_HANDLE };

        cString name{};

    };
}

#endif // !VK_PIPELIN_INCLUDE_H_
