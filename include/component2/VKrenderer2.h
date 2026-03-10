#ifndef VK_RENDERER_INCLUDE_2_H_
#define VK_RENDERER_INCLUDE_2_H_

#include "data.h"
#include "ubo.h"

#include "Camera2.h"
#include "VKDescriptorSet.h"
#include "VKContext.h"
#include "VKImage2D.h"
#include "VKStorageBuffer.h"
#include "VKSamplerHandler.h"
#include "VKSkyTexture.h"
#include "VKpipeLineHandle.h"
#include "VKDepthStencil.h"
#include "VKViewFrustum.h"
#include "VKModel.h"
#include "VKUniformBuffer2.h"
#include "VKShaderManager.h"

#include "VKRenderGraph.h"
#include "VKswapchain.h"

namespace vkengine
{

    struct CullingStats
    {
        cUint32_t totalMeshes = 0;
        cUint32_t culledMeshes = 0;
        cUint32_t renderedMeshes = 0;
    };

    class VKforwardRenderer2
    {
    public:
        // 렌더러 생성자 - Vulkan 컨텍스트, 셰이더 매니저, 프레임 수, 리소스 경로 초기화
        VKforwardRenderer2(VKcontext &ctx, VKShaderManager &shadermanager,
                           const cUint32_t &MaxFramesFlight,
                           const cString &assetsPath,
                           const cString &shaderPath);

        ~VKforwardRenderer2();
        void cleanup();

        void rendering(
            VkCommandBuffer cmd,
            uint32_t currentFrame,
            std::vector<VKModel> &models,
            VkViewport viewport,
            VkRect2D scissor);

        void buildRenderGraph(VKSwapChain &swapchain);

        void prepareForModels(
            std::vector<VKModel> &models,
            VkFormat outColorFormat,
            VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples,
            cUint32_t swapChainWidth,
            cUint32_t swapChainHeight);

    private:
        const cUint32_t MaxFramesFlight;
        const cString assetsPath;
        const cString shaderPath;

        VKcontext &ctx;
        VKShaderManager &shaderManager;
        VKRenderGraph renderGraph;

        // 파이프 라인 핸들 매핑 - 파이프라인 이름으로 핸들 관리
        std::unordered_map<cString, VKPipeLineHandle> pipelines;

        // Resources
        // 랜더 타겟, 텍스처, 샘플러, 스카이박스 IBL 텍스처
        std::vector<VKModel> *currentModels;
        VkViewport currentViewport;
        VkRect2D currentScissor;

        SceneDataUBO sceneDataUBO;
        SkyOptionsUBO skyOptionsUBO;
        OptionsUniform optionsUBO;
        BoneDataUniform boneDataUBO;
        PostProcessingOptionsUBO postOptionsUBO;

        std::vector<VKUniformBuffer2<SceneDataUBO>> sceneDataUniform;
        std::vector<VKUniformBuffer2<SkyOptionsUBO>> skyOptionsUniform;
        std::vector<VKUniformBuffer2<OptionsUniform>> optionsUniform;
        std::vector<VKUniformBuffer2<BoneDataUniform>> boneDataUniform;
        std::vector<VKUniformBuffer2<PostProcessingOptionsUBO>> postOptionsUniform;

        std::vector<DescriptorSetHander> SceneSkyOptionsStates{};
        std::vector<DescriptorSetHander> SceneOptionsBoneDataSets{};
        std::vector<DescriptorSetHander> PostDescriptorSets{};

        VKImage2D msaaColorBuffer;
        VKDepthStencil depthStencil;
        VKDepthStencil msaaDepthStencil;

        VKImage2D forwardToCompute;
        VKImage2D computeToPost;

        VKImage2D dummyTexture;
        VKskyTexture skyTextures;
        VKImage2D shadowMap;

        VKSamplerHandler samplerLinearRepeat;
        VKSamplerHandler samplerLinearClamp;
        VKSamplerHandler samplerAnisoRepeat;
        VKSamplerHandler samplerAnisoClamp;
        VKSamplerHandler samplerShadowMap;

        DescriptorSetHander skyDescriptorSet;
        // DescriptorSetHander postDescriptorSet;
        DescriptorSetHander shadowMapSet;

        // 랜더링을 위한 리소스 생성 함수들
        // PBR, 스카이박스, 포스트 프로세싱, 쉐도우 맵 파이프라인 생성
        // 렌더 타겟, 텍스처, 샘플러, 스카이박스 IBL 텍스처 생성
        // Scene, Sky, Options, BoneData, PostProcessing 유니폼 버퍼 및 디스크립터 셋 생성
        void createPipelines(const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits msaaSamples);
        void createTextures(cUint32_t swapchainWidth, cUint32_t swapchainHeight, VkSampleCountFlagBits msaaSamples);
        void createUniformBuffers();

        void makeShadowMap(VkCommandBuffer cmd, cUint32_t currentFrame);
        void makeForwardPBRPass(VkCommandBuffer cmd, cUint32_t currentFrame);
        void makeSkyPass(VkCommandBuffer cmd, cUint32_t currentFrame);
        void makePostProcessPass(VkCommandBuffer cmd, cUint32_t currentFrame);
        void makePresent(VkCommandBuffer cmd, cUint32_t currentFrame);

        // Helper functions for creating rendering structures
        // 컬러 어태치먼트 정보 생성 - MSAA resolve 지원
        VkRenderingAttachmentInfo
        createColorAttachment(VkImageView imageView,
                              VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                              VkClearColorValue clearColor = {0.0f, 0.0f, 0.0f, 0.0f},
                              VkImageView resolveImageView = VK_NULL_HANDLE,
                              VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE) const;

        // 깊이 어태치먼트 정보 생성 - MSAA resolve 지원
        VkRenderingAttachmentInfo
        createDepthAttachment(VkImageView imageView,
                              VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                              float clearDepth = 1.0f, VkImageView resolveImageView = VK_NULL_HANDLE,
                              VkResolveModeFlagBits resolveMode = VK_RESOLVE_MODE_NONE) const;

        // 렌더링 정보 구조체 생성 - 동적 렌더링에 사용
        VkRenderingInfo
        createRenderingInfo(const VkRect2D &renderArea,
                            const VkRenderingAttachmentInfo *colorAttachment,
                            const VkRenderingAttachmentInfo *depthAttachment = nullptr) const;
    };
}

#endif // !VK_RENDERER_INCLUDE_H_
