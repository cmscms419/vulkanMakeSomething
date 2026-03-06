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

        void buildRenderGraph(VKSwapChain &swapchain);

    private:
        const cUint32_t MaxFramesFlight = MAX_FRAMES_IN_FLIGHT;
        const cString assetsPath = RESOURSE_PATH;
        const cString shaderPath = SHADER_PATH;

        VKcontext &ctx;
        VKShaderManager &shaderManager;
        VKRenderGraph renderGraph;

        // Resources
        // 렌더 상태 (람다 캡처용)
        std::vector<VKModel> *currentModels = nullptr;
        VkViewport currentViewport{};
        VkRect2D currentScissor{};
        VkImageView currentSwapchainImageView = VK_NULL_HANDLE;

        // 각 패스 실행 함수 (람다에서 호출)
        void executeShadowPass(VkCommandBuffer cmd, cUint32_t frameIndex);
        void executeForwardPass(VkCommandBuffer cmd, cUint32_t frameIndex);
        void executePostPass(VkCommandBuffer cmd, cUint32_t frameIndex);

        // 쉐도우 맵 생성 - 광원 시점에서 깊이 정보 렌더링
        void makeShadowMap(VkCommandBuffer cmd, uint32_t currentFrame);
    };
}

#endif // !VK_RENDERER_INCLUDE_H_
