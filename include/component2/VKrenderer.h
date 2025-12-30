#ifndef VK_RENDERER_INCLUDE_H_
#define VK_RENDERER_INCLUDE_H_

#include "log.h"
#include "resourseload.h"

#include "Camera2.h"
#include "VKDescriptorSet.h"
#include "VKContext.h"
#include "VKImage2D.h"
#include "VKStorageBuffer.h"
#include "VKSampler.h"
#include "VKSkyTexture.h"
#include "pipeLineHandle.h"
#include "VKDepthStencil.h"
#include "VKViewFrustum.h"
#include "VKModel.h"
#include "VKUniformBuffer2.h"
#include "VKShaderManager.h"
#include "VKShadowMap.h"

namespace vkengine {

    struct CullingStats
    {
        cUint32_t totalMeshes = 0;
        cUint32_t culledMeshes = 0;
        cUint32_t renderedMeshes = 0;
    };

    class VKforwardRenderer {

    public:
        // 렌더러 생성자 - Vulkan 컨텍스트, 셰이더 매니저, 프레임 수, 리소스 경로 초기화
        VKforwardRenderer(VKcontext& ctx, VKShaderManager& shadermanager,
            const cUint32_t& MaxFramesFlight,
            const cString& assetsPath,
            const cString& shaderPath);

        ~VKforwardRenderer();

        // 모델 렌더링을 위한 전체 준비 과정 수행 (파이프라인, 텍스처, 유니폼 버퍼 생성 및 모델 디스크립터 설정)
        void prepareForModels(std::vector<VKModel>& models, VkFormat outColorFormat, VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples, uint32_t swapChainWidth,
            uint32_t swapChainHeight);

        // PBR, 스카이박스, 포스트 프로세싱, 쉐도우 맵 파이프라인 생성
        void createPipelines(const VkFormat colorFormat, const VkFormat depthFormat,
            VkSampleCountFlagBits msaaSamples);
        // 렌더 타겟, 텍스처, 샘플러, 스카이박스 IBL 텍스처 생성
        void createTextures(uint32_t swapchainWidth, uint32_t swapchainHeight,
            VkSampleCountFlagBits msaaSamples);
        // Scene, Sky, Options, BoneData, PostProcessing 유니폼 버퍼 및 디스크립터 셋 생성
        void createUniformBuffers();

        void cleanup()
        {
            // Manual cleanup is not necessary
        }

        // 현재 프레임의 유니폼 버퍼 데이터 업데이트 (Scene, Options, Sky, PostProcessing)
        void update(object::Camera2& camera, uint32_t currentFrame, double time);
        // 애니메이션이 있는 모델의 본 행렬 데이터를 유니폼 버퍼에 업데이트
        void updateBoneData(const std::vector<VKModel>& models, uint32_t currentFrame);

        // Forward rendering pass 수행 - MSAA 컬러 버퍼에 모델 렌더링 후 스카이박스 렌더링
        void draw(VkCommandBuffer cmd, uint32_t currentFrame, VkImageView swapchainImageView,
            std::vector<VKModel>& models, VkViewport viewport, VkRect2D scissor);

        // 쉐도우 맵 생성 - 광원 시점에서 깊이 정보 렌더링
        void makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame, std::vector<VKModel>& models);

        // View frustum culling
        // 컬링 통계 정보 반환 (전체, 컬링된, 렌더링된 메시 수)
        auto getCullingStats() const -> const CullingStats&;
        // 프러스텀 컬링 활성화 상태 반환
        bool isFrustumCullingEnabled() const;
        // 뷰 프러스텀 기반 메시 컬링 수행 - 보이지 않는 메시는 isCulled 플래그 설정
        void performFrustumCulling(std::vector<VKModel>& models);
        // 프러스텀 컬링 활성화/비활성화 설정
        void setFrustumCullingEnabled(bool enabled);
        // 뷰 프로젝션 행렬로부터 프러스텀 평면 추출 및 업데이트
        void updateViewFrustum(const cMat4& viewProjection);



        // UBO getter 함수들 - 각 유니폼 버퍼 오브젝트의 현재 데이터 반환
        SceneDataUBO& getSceneDataUBO() {
            return this->sceneDataUBO;
        }
        SkyOptionsUBO& getSkyOptionsUBO() {
            return this->skyOptionsUBO;
        }
        OptionsUniform& getOptionsUniform() {
            return this->optionsUBO;
        }
        BoneDataUniform& getBoneDataUniform() {
            return this->boneDataUBO;
        }
        PostProcessingOptionsUBO& getPostProcessingOptionsUBO() {
            return this->postOptionsUBO;
        }

    private:
        const cUint32_t MaxFramesFlight = MAX_FRAMES_IN_FLIGHT;
        const cString assetsPath = RESOURSE_PATH;
        const cString shaderPath = SHADER_PATH;

        VKcontext& ctx;
        VKShaderManager& shaderManager;

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

        // Resources
        VKImage2D msaaColorBuffer;
        VKDepthStencil depthStencil;
        VKDepthStencil msaaDepthStencil;

        VKImage2D forwardToCompute;
        VKImage2D computeToPost;

        VKImage2D dummyTexture;
        VKskyTexture skyTextures;

        VKSampler samplerLinearRepeat;
        VKSampler samplerLinearClamp;
        VKSampler samplerAnisoRepeat;
        VKSampler samplerAnisoClamp;

        VKShadowMap shadowMap;

        DescriptorSetHander skyDescriptorSet;
        DescriptorSetHander postDescriptorSet;
        DescriptorSetHander shadowMapSet;

        std::unordered_map<cString, PipeLineHandle> pipelines;

        ViewFrustum viewFrustum{};
        cBool frustumCullingEnabled{ true };

        // Statistics
        CullingStats cullingStats;

        // Control parameters
        cFloat directionalLightAngle1 = 27.0f;
        cFloat directionalLightAngle2 = 3.0f;
        cFloat directionalLightIntensity = 27.66f;

        // Shadow mapping bias parameters for real-time adjustment
        cFloat shadowBiasConstant = 0.5f; // Constant bias factor
        cFloat shadowBiasSlope = 1.0f;    // Slope-scaled bias factor
        cFloat shadowBiasClamp = 0.0f;    // Bias clamp value

        // SSAO parameters
        cFloat ssaoRadius = 0.5f;
        cFloat ssaoBias = 0.025f;
        cInt ssaoSampleCount = 16;
        cFloat ssaoPower = 2.0f;

        // Helper functions for creating rendering structures
        // 컬러 어태치먼트 정보 생성 - MSAA resolve 지원
        VkRenderingAttachmentInfo
            createColorAttachment(VkImageView imageView,
                VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                VkClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f },
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
            createRenderingInfo(const VkRect2D& renderArea,
                const VkRenderingAttachmentInfo* colorAttachment,
                const VkRenderingAttachmentInfo* depthAttachment = nullptr) const;

    };
}

#endif // !VK_RENDERER_INCLUDE_H_
