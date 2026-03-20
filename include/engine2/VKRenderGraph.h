#ifndef VK_INCLUDE_RENDER_GRAPH_H
#define VK_INCLUDE_RENDER_GRAPH_H

#include "vkconfig.h"

#include "VKImage2D.h"
#include "VKswapchain.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace vkengine
{
    // Swapchain은 특별 취급이 필요하므로, 리소스 핸들 중 하나는 swapchain으로 예약되어 있다고 가정
    struct ResourceEntry
    {
        VKImage2D *image = nullptr;
        VKSwapChain *swapchain = nullptr;
    };

    using PassExecuteFunc = std::function<void(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex)>;

    struct RenderPassNode
    {
        cString name;
        std::vector<ResourceUsage> inputs;  // 이 패스가 읽는 리소스
        std::vector<ResourceUsage> outputs; // 이 패스가 쓰는 리소스
        std::vector<ResourceUsage> shaderResources; // 디스크립터 바인딩 용
    };

    class VKRenderGraph
    {
    public:
        explicit VKRenderGraph(VKcontext &ctx);

        void registerResource(const cString &handle, VKImage2D &img);
        void registerSwapchainResource(const cString &handle, VKSwapChain &swapchain);
        void registerPassFunction(const cString& name, PassExecuteFunc func);
        void addPass(RenderPassNode pass);
        bool compile(); // addPass 후 한 번만 호출 (위상 정렬 + 유효성 검사)
        void execute(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex);
        bool loadFromJson(const cString& filePath);

        VKSwapChain& getVKSwapChain();

    private:
        VKcontext &ctx;
        std::unordered_map<cString, ResourceEntry> resources;
        std::unordered_map<cString, PassExecuteFunc> passRegistry;
        cString swapchainHandle;
        std::vector<RenderPassNode> passes;
        std::vector<cSize> sortedOrder; // compile()이 채워넣음

        std::vector<std::vector<cSize>> buildAdjacency() const;
        std::vector<cSize> topologicalSort(const std::vector<std::vector<cSize>> &adj) const;
        void insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t imageindex, const RenderPassNode &pass);
        void applyBarrier(VkCommandBuffer cmd, cUint32_t imageindex, const ResourceUsage &res, const cString &passName);
        void printGraph() const;
    };

} // namespace vkengine

#endif // VK_INCLUDE_RENDER_GRAPH_H