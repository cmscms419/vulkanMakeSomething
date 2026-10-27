#include "RenderGraph.h"
#include "log.h"

#include "../../external/tinygltf/json.hpp"

using json = nlohmann::json;

namespace vkengine
{
    RenderGraph::RenderGraph(Context &ctx, SwapChain &swapchain)
     : ctx(ctx), swapchain(swapchain)
    {
    }

    void RenderGraph::registerResource(const cString &handle, std::shared_ptr<Image2D> image)
    {
        if (resources.find(handle) != resources.end())
        {
            Log::printToLogger("Warning: Resource '%s' is already registered in RenderGraph. Overwriting.", handle.c_str());
            return;
        }

        ResourceEntry entry;
        entry.image = image;

        resources.emplace(handle, entry);
    }

    void RenderGraph::registerPassFunction(const cString &name, PassExecuteFunc func)
    {
        if (this->passRegistry.find(name) != this->passRegistry.end())
        {
            Log::printToLogger("Warning: function '%s' is already registered in RenderGraph. Overwriting.", name.c_str());
            return;
        }

        passRegistry.emplace(name, func);
    }

    void RenderGraph::addPass(RenderPassNode pass)
    {
        passes.push_back(std::move(pass));
    }

    bool RenderGraph::compile()
    {
        // 유효성 검사: shaderResources에 등록된 리소스가 실제로 존재하는지 확인
        for (const auto &pass : passes)
        {
            for (const auto &res : pass.shaderResources)
            {
                if (resources.find(res.handle) == resources.end())
                {
                    Log::exitToLogger("Error: Pass '%s' uses unregistered resource '%s'.", pass.name.c_str(), res.handle.c_str());
                }
            }
        }

        printGraph();

        return true;
    }

    void RenderGraph::execute(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex)
    {
        for (const RenderPassNode &pass : passes)
        {
            insertBarriersBeforePass(cmd, imageindex, pass);

            if (this->passRegistry.find(pass.name) != this->passRegistry.end())
            {
                this->passRegistry[pass.name](cmd, frameIndex, imageindex);
            }   
        }
    }

    void RenderGraph::printGraph() const
    {
        Log::printToLogger("RenderGraph Execution Order:");
        for (const RenderPassNode &pass : passes)
        {
            Log::printToLogger("Pass: %s", pass.name.c_str());
            for (const auto &res : pass.shaderResources)
            {
                Log::printToLogger("  Resource: %s (%s)", res.handle.c_str(), getStringResourceAccess(res.access).c_str());
            }
        }
    }

    // 샘플링 읽기(ShaderReadOnly)는 그래픽 패스(FRAGMENT)와 컴퓨트 패스(COMPUTE) 양쪽에서 일어난다.
    // dstStageMask 를 실제보다 넓게 잡는 것은 과동기화일 뿐 항상 안전하므로,
    // 패스 종류를 따로 받지 않고 두 스테이지를 함께 준다.
    // 다만 추후 문제가 발생시 각 패스 종류를 구분하여 dstStageMask 를 좁게 잡는 방법을 고려할 수 있다.
    static BarrierHelperFunction::BarrierParams toBarrierParams(ResourceAccess access)
    {
        switch (access)
        {
        case ResourceAccess::ColorAttachmentWrite:
            return { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
        case ResourceAccess::DepthAttachmentWrite:
            return { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                     VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT };
        case ResourceAccess::ShaderReadOnly:
            return { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_ACCESS_2_SHADER_READ_BIT,
                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT };
        case ResourceAccess::StorageRead:
            return { VK_IMAGE_LAYOUT_GENERAL,
                     VK_ACCESS_2_SHADER_READ_BIT,
                     VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT };
        case ResourceAccess::ShaderWriteOnly:
            return { VK_IMAGE_LAYOUT_GENERAL,
                     VK_ACCESS_2_SHADER_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT };
        case ResourceAccess::ShaderReadWrite:
            return { VK_IMAGE_LAYOUT_GENERAL,
                     VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT };
        case ResourceAccess::Present:
            return { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                     VK_ACCESS_2_NONE,
                     VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT };
        default:
            return { VK_IMAGE_LAYOUT_UNDEFINED, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_NONE };
        }
    }

    void RenderGraph::insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t imageindex, const RenderPassNode &pass)
    {
        std::vector<BarrierHelperFunction::TransitionRequest> requests;

        for (const ResourceUsage& res : pass.shaderResources)
        {
            auto it = resources.find(res.handle);
            if (it == resources.end()) continue;

            ResourceEntry& entry = it->second;
            if (entry.image == nullptr) continue;

            if (res.access == ResourceAccess::None) continue;

            requests.push_back({
                entry.image.get(),
                toBarrierParams(res.access)
            });
        }

        BarrierHelperFunction::batchTransition(cmd, requests);
    }

    bool RenderGraph::loadFromJSON(const cString &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            return false;
        }

        json root;
        root = json::parse(file);

        for (const auto &passJson : root["passes"])
        {
            RenderPassNode node;
            cString name = passJson["name"];

            node.name = name;

            for (const auto &item : passJson["shaderResources"])
            {
                cString handle = item["handle"];
                cString access = item["access"];

                node.shaderResources.push_back({handle, getResourceAccess(access)});
            }

            this->passes.push_back(std::move(node));
        }

        return true;
    }

    SwapChain &RenderGraph::getSwapChain()
    {
        return this->swapchain;
    }
}