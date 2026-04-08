#ifndef VK_INCLUDE_RENDER_GRAPH_H
#define VK_INCLUDE_RENDER_GRAPH_H

#include "vkconfig.h"

#include "VKImage2D.h"
#include "VKswapchain.h"
#include "VKbuffer2.h"

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

namespace vkengine
{
    // Swapchain은 특별 취급이 필요하므로, 리소스 핸들 중 하나는 swapchain으로 예약되어 있다고 가정
    // 나중에 필요하면, buffer 리소스도 추가할 수 있지만, 일단은 이미지 리소스만 관리한다고 가정
    struct ResourceEntry
    {
        std::shared_ptr<VKImage2D> image;
        std::shared_ptr<VKBaseBuffer2> buffer; // 향후 확장 가능
    };

    using PassExecuteFunc = std::function<void(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex)>;

    struct RenderPassNode
    {
        cString name;
        std::vector<ResourceUsage> shaderResources;
    };

    class VKRenderGraph
    {
    public:
        explicit VKRenderGraph(VKcontext &ctx, VKSwapChain &swapchain);

        void registerResource(const cString &handle, std::shared_ptr<VKImage2D> image);
        void registerPassFunction(const cString& name, PassExecuteFunc func);
        void addPass(RenderPassNode pass);
        bool compile(); // addPass 후 한 번만 호출 (유효성 검사)
        void execute(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex);
        bool loadFromJson(const cString& filePath);

        VKSwapChain& getVKSwapChain();

    private:
        VKcontext &ctx;
        VKSwapChain &swapchain;
        std::unordered_map<cString, ResourceEntry> resources;
        std::unordered_map<cString, PassExecuteFunc> passRegistry;
        std::vector<RenderPassNode> passes;

        void insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t imageindex, const RenderPassNode &pass);
        void printGraph() const;
    };

} // namespace vkengine

#endif // VK_INCLUDE_RENDER_GRAPH_H