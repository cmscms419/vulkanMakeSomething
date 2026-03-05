#include "VKRenderGraph.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    RenderGraph::RenderGraph(VKcontext &ctx) : ctx(ctx)
    {
    }
    void RenderGraph::registerResource(const cString &handle, VKImage2D &img)
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

    void RenderGraph::registerSwapchainResource(const cString &handle, VKSwapChain &swapchain)
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
    }

    void RenderGraph::addPass(RenderPassNode pass)
    {
        passes.push_back(std::move(pass));
    }

    bool RenderGraph::compile()
    {
        cBool result = false;

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
        std::vector<std::vector<cSize>> adj = buildAdjacency();

        // 3. topologicalSort() → sortedOrder
        sortedOrder = topologicalSort(adj);

        // 4. printGraph() 로그 출력 (선택)

        return result;
    }

    void RenderGraph::execute(VkCommandBuffer cmd, cUint32_t frameIndex, VkImage swapchainImage, VkImageView swapchainView)
    {
    }

    void RenderGraph::printGraph() const
    {
    }

    std::vector<std::vector<cSize>> RenderGraph::buildAdjacency() const
    {
        std::vector<std::vector<cSize>> adj;
        std::unordered_map<cString, std::vector<cSize>> outputResources; // 리소스 핸들 → 패스 인덱스 매핑 (출력 리소스 기준)

        adj.resize(passes.size());
        outputResources.reserve(passes.size() * 4); // 패스당 평균 4개의 출력 리소스 가정

        for (cSize i = 0; i < passes.size(); ++i)
        {
            for (const auto &res : passes[i].outputs)
            {
                outputResources[res.handle].push_back(i); // 이 해들이 어떤 인풋의 인덱스 인지 저장, output에 해당하는 i 값이 패스 인덱스
            }
        }

        for (cSize i = 0; i < passes.size(); ++i)
        {
            for (const auto &res : passes[i].inputs)
            {
                auto it = outputResources.find(res.handle);

                if (it != outputResources.end())
                {
                    std::vector<cSize> &srcs = outputResources[res.handle];

                    for (cSize src : srcs)
                    {
                        if (src != i) // 자기 자신 패스는 제외
                        {
                            adj[src].push_back(i); // src 패스 → i 패스 간선 추가
                        }
                    }
                }
                else
                {
                    EXIT_TO_LOGGER("Error: Pass '%s' reads from resource '%s' that is not produced by any pass.", passes[i].name.c_str(), res.handle.c_str());
                }
            }
        }

        return adj;
    }

    std::vector<cSize> RenderGraph::topologicalSort(const std::vector<std::vector<cSize>> &adj) const
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

    void RenderGraph::insertBarriersBeforePass(VkCommandBuffer cmd, const RenderPassNode &pass)
    {
    }

    VKBarrierHelper RenderGraph::toBarrierHelper(ResourceAccess access, VkFormat format)
    {
        return VKBarrierHelper();
    }
}