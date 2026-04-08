#include "VKRenderGraph.h"
#include "log.h"

#include "../../external/tinygltf/json.hpp"

using namespace vkengine::Log;
using json = nlohmann::json;

namespace vkengine
{
    VKRenderGraph::VKRenderGraph(VKcontext &ctx, VKSwapChain &swapchain)
     : ctx(ctx), swapchain(swapchain)
    {
    }

    void VKRenderGraph::registerResource(const cString &handle, std::shared_ptr<VKImage2D> image)
    {
        if (resources.find(handle) != resources.end())
        {
            PRINT_TO_LOGGER("Warning: Resource '%s' is already registered in RenderGraph. Overwriting.", handle.c_str());
            return;
        }

        ResourceEntry entry;
        entry.image = image;

        resources.emplace(handle, entry);
    }

    void VKRenderGraph::registerPassFunction(const cString &name, PassExecuteFunc func)
    {
        if (this->passRegistry.find(name) != this->passRegistry.end())
        {
            PRINT_TO_LOGGER("Warning: function '%s' is already registered in RenderGraph. Overwriting.", name.c_str());
            return;
        }

        passRegistry.emplace(name, func);
    }

    void VKRenderGraph::addPass(RenderPassNode pass)
    {
        passes.push_back(std::move(pass));
    }

    bool VKRenderGraph::compile()
    {
        // 유효성 검사: shaderResources에 등록된 리소스가 실제로 존재하는지 확인
        for (const auto &pass : passes)
        {
            for (const auto &res : pass.shaderResources)
            {
                if (resources.find(res.handle) == resources.end())
                {
                    EXIT_TO_LOGGER("Error: Pass '%s' uses unregistered resource '%s'.", pass.name.c_str(), res.handle.c_str());
                }
            }
        }

        printGraph();

        return true;
    }

    void VKRenderGraph::execute(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex)
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

    void VKRenderGraph::printGraph() const
    {
        PRINT_TO_LOGGER("RenderGraph Execution Order:");
        for (const RenderPassNode &pass : passes)
        {
            PRINT_TO_LOGGER("Pass: %s", pass.name.c_str());
            for (const auto &res : pass.shaderResources)
            {
                PRINT_TO_LOGGER("  Resource: %s (%s)", res.handle.c_str(), getStringResourceAccess(res.access).c_str());
            }
        }
    }

    // ResourceAccess enum → VKBarrierHelperFunction::BarrierParams 변환
    // transitionToXxx()에 하드코딩됐던 값들을 한 곳에 집중
    static VKBarrierHelperFunction::BarrierParams toBarrierParams(ResourceAccess access)
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
                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT };
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

    void VKRenderGraph::insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t imageindex, const RenderPassNode &pass)
    {
        std::vector<VKBarrierHelperFunction::TransitionRequest> requests;

        for (const ResourceUsage& res : pass.shaderResources)
        {
            auto it = resources.find(res.handle);
            if (it == resources.end()) continue;

            ResourceEntry& entry = it->second;
            if (entry.image == nullptr) continue;

            if (res.access == ResourceAccess::NOTTHING) continue;

            requests.push_back({
                entry.image->getImage(),
                &entry.image->getBarrierHelper(),
                toBarrierParams(res.access)
            });
        }

        VKBarrierHelperFunction::batchTransition(cmd, requests);
    }

    bool VKRenderGraph::loadFromJson(const cString &filePath)
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

    VKSwapChain &VKRenderGraph::getVKSwapChain()
    {
        return this->swapchain;
    }
}