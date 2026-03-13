#include "VKRenderGraph.h"
#include "log.h"

#include <queue>

using namespace vkengine::Log;

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

    void VKRenderGraph::addPass(RenderPassNode pass)
    {
        passes.push_back(std::move(pass));
    }

    bool VKRenderGraph::compile()
    {
        // 1. 유효성 검사
        for (const auto &pass : passes)
        {
            for (const auto &res : pass.inputs)
            {
                if (resources.find(res.handle) == resources.end())
                {
                    EXIT_TO_LOGGER("Error: Pass '%s' reads from unregistered resource '%s'.", pass.name.c_str(), res.handle.c_str());
                }
            }
            for (const auto &res : pass.outputs)
            {
                if (resources.find(res.handle) == resources.end())
                {
                    EXIT_TO_LOGGER("Error: Pass '%s' writes to unregistered resource '%s'.", pass.name.c_str(), res.handle.c_str());
                }
            }
        }
        // 2. buildAdjacency()
        // "어떤 패스가 끝나야 어떤 패스가 시작될 수 있는지"를 인덱스로 표현한 인접 리스트 생성
        std::vector<std::vector<cSize>> adj = buildAdjacency();

        // 3. topologicalSort() → sortedOrder
        // 실제 실행 순서를 결정하는 위상 정렬 알고리즘 (Kahn's algorithm)
        sortedOrder = topologicalSort(adj);

        // 4. printGraph() 로그 출력 (선택)
        printGraph();

        return true;
    }

    void VKRenderGraph::execute(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex)
    {
        for (const cSize index : sortedOrder)
        {
            const RenderPassNode &pass = passes[index];

            // 각 패스 실행 전에 필요한 배리어 삽입
            insertBarriersBeforePass(cmd, frameIndex, imageindex, pass);

            // 패스 실행
            pass.execute(cmd, frameIndex, imageindex);
        }
    }

    VKSwapChain &VKRenderGraph::getVKSwapChain()
    {
        auto result = this->resources.find(this->swapchainHandle);

        return *result->second.swapchain;
    }

    void VKRenderGraph::printGraph() const
    {
#if 1
        PRINT_TO_LOGGER("RenderGraph Execution Order:");
        for (cSize idx : sortedOrder)
        {
            const RenderPassNode &pass = passes[idx];

            PRINT_TO_LOGGER("Pass: %s", pass.name.c_str());
            for (const auto &res : pass.inputs)
            {
                PRINT_TO_LOGGER("  Input: %s (%s)", res.handle.c_str(), getStringResourceAccess(res.access).c_str());
            }
            for (const auto &res : pass.outputs)
            {
                PRINT_TO_LOGGER("  Output: %s (%s)", res.handle.c_str(), getStringResourceAccess(res.access).c_str());
            }
        }
#endif
        return;
    }

    std::vector<std::vector<cSize>> VKRenderGraph::buildAdjacency() const
    {
        std::vector<std::vector<cSize>> adj;
        std::unordered_map<cString, std::vector<cSize>> outputResources; // 리소스 핸들 → 패스 인덱스 매핑 (출력 리소스 기준)

        adj.reserve(passes.size());
        adj.resize(passes.size());

        outputResources.reserve(passes.size() * static_cast<cSize>(ResourceAccess::MAX)); // 패스당 최대 ResourceAccess::MAX 개의 리소스가 있다고 가정하고 예약

        // 모든 pass의 outputs 리소스 핸들을 outputResources에 등록
        for (cSize i = 0; i < passes.size(); ++i)
        {
            for (const auto &res : passes[i].outputs)
            {
                outputResources[res.handle].push_back(i); // 이 해들이 어떤 인풋의 인덱스 인지 저장, output에 해당하는 i 값이 패스 인덱스
            }
        }

        // 모든 pass의 inputs 리소스 핸들을 outputResources에서 찾아서 간선 추가
        for (cSize i = 0; i < passes.size(); ++i)
        {
            for (const auto &res : passes[i].inputs)
            {
                auto it = outputResources.find(res.handle);

                if (it != outputResources.end())
                {
                    std::vector<cSize> &srcs = outputResources[res.handle]; // passes[i].inputs이 어떤 패스의 output인지 srcs에 저장

                    for (cSize src : srcs)
                    {
                        adj[src].push_back(i); // src 패스 → i 패스 간선 추가
                    }
                }
            }
        }

        return adj;
    }

    std::vector<cSize> VKRenderGraph::topologicalSort(const std::vector<std::vector<cSize>> &adj) const
    {
        std::vector<cSize> sorted;
        std::vector<cSize> inDegree(passes.size(), 0);

        // 1. inDegree 계산 — 각 패스로 들어오는 엣지 수
        for (cSize i = 0; i < adj.size(); ++i)
            for (cSize j : adj[i])
                inDegree[j]++;

        // 2. inDegree == 0 (의존성 없음) 인 패스를 큐에 넣기
        std::queue<cSize> q;
        for (cSize i = 0; i < passes.size(); ++i)
            if (inDegree[i] == 0)
                q.push(i);

        // 3. BFS
        while (!q.empty())
        {
            cSize cur = q.front();
            q.pop();
            sorted.push_back(cur);

            for (cSize next : adj[cur])
                if (--inDegree[next] == 0)
                    q.push(next);
        }

        // 4. 사이클 감지
        if (sorted.size() != passes.size())
            EXIT_TO_LOGGER("Error: Cycle detected in RenderGraph.");

        return sorted;
    }

    void VKRenderGraph::insertBarriersBeforePass(VkCommandBuffer cmd, cUint32_t frameIndex, cUint32_t imageindex, const RenderPassNode &pass)
    {
        // 패스의 입력 리소스들을 순회하면서 필요한 배리어 삽입
        for (const ResourceUsage &res : pass.inputs)
        {
            ResourceEntry &entry = resources[res.handle];

            if (entry.image == nullptr && entry.swapchain == nullptr)
            {
                PRINT_TO_LOGGER("Error: Resource '%s' used in pass '%s' is not registered as either image or swapchain.", res.handle.c_str(), pass.name.c_str());
                continue;
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
            case ResourceAccess::Present: // 아직 VKimage2D로 출력하는 구문을 만들지 않음
                entry.swapchain->transitionTo(cmd, frameIndex);
                break;
            default:
                break;
            }
        }

        // 패스의 출력 리소스들을 순회하면서 필요한 배리어 삽입
        for (const ResourceUsage &res : pass.outputs)
        {
            ResourceEntry &entry = resources[res.handle];

            if (entry.image == nullptr && entry.swapchain == nullptr)
            {
                PRINT_TO_LOGGER("Error: Resource '%s' used in pass '%s' is not registered as either image or swapchain.", res.handle.c_str(), pass.name.c_str());
                continue;
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
            case ResourceAccess::Present:
                entry.swapchain->transitionTo(cmd, imageindex);
                break;
            default:
                break;
            }
        }
    }
}