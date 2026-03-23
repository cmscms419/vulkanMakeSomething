#include "VKRenderGraph.h"
#include "log.h"

#include "../../external/tinygltf/json.hpp"

using namespace vkengine::Log;
using json = nlohmann::json;

namespace vkengine
{
    VKRenderGraph::VKRenderGraph(VKcontext &ctx) : ctx(ctx)
    {
    }
    void VKRenderGraph::registerResource(const cString &handle, VKImage2D &img)
    {
        if (resources.find(handle) != resources.end())
        {
            PRINT_TO_LOGGER("Warning: Resource '%s' is already registered in RenderGraph. Overwriting.", handle.c_str());
            return;
        }
        ResourceEntry entry;

        entry.image = &img;
        entry.swapchain = nullptr;

        resources.emplace(handle, entry);
    }

    void VKRenderGraph::registerSwapchainResource(const cString &handle, VKSwapChain &swapchain)
    {
        if (resources.find(handle) != resources.end())
        {
            PRINT_TO_LOGGER("Warning: Resource '%s' is already registered in RenderGraph. Overwriting.", handle.c_str());
            return;
        }
        ResourceEntry entry;

        entry.image = nullptr;
        entry.swapchain = &swapchain;

        resources.emplace(handle, entry);

        swapchainHandle = handle;
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

    VKSwapChain &VKRenderGraph::getVKSwapChain()
    {
        auto result = this->resources.find(this->swapchainHandle);

        return *result->second.swapchain;
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

    void VKRenderGraph::applyBarrier(VkCommandBuffer cmd, cUint32_t imageindex, const ResourceUsage &res, const cString &passName)
    {
        ResourceEntry &entry = resources[res.handle];

        if (entry.image == nullptr && entry.swapchain == nullptr)
        {
            PRINT_TO_LOGGER("Error: Resource '%s' used in pass '%s' is not registered as either image or swapchain.", res.handle.c_str(), passName.c_str());
            return;
        }

        switch (res.access)
        {
        case ResourceAccess::ColorAttachmentWrite:
            entry.image->transitionToColorAttachment(cmd);
            break;
        case ResourceAccess::DepthAttachmentWrite:
            entry.image->transitionToDepthStencilAttachment(cmd);
            break;
        case ResourceAccess::ShaderReadOnly:
            entry.image->transitionToShaderReadOnly(cmd);
            break;
        case ResourceAccess::ShaderReadWrite:
            entry.image->transitionToShaderReadWrite(cmd);
            break;
        case ResourceAccess::ShaderWriteOnly:
            entry.image->transitionToShaderWriteOnly(cmd);
            break;
        case ResourceAccess::Present:
            entry.swapchain->transitionTo(cmd, imageindex);
            break;
        default:
            break;
        }
    }

    void VKRenderGraph::insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t imageindex, const RenderPassNode &pass)
    {
        for (const ResourceUsage &res : pass.shaderResources)
            applyBarrier(cmd, imageindex, res, pass.name);
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

}