#include "DataManagement.h"

#include "log.h"
#include "vkconfig.h"

#include <cstring>

namespace vkengine
{
    ClothDataManagement::ClothDataManagement(Context& ctx, ShaderManager& shaderManager)
        : ctx(ctx), shaderManager(shaderManager),
          predictPipeline(ctx, shaderManager),
          solvePipeline(ctx, shaderManager),
          finalizePipeline(ctx, shaderManager),
          paramsBuffer(ctx),
          vertexBuffer(ctx),
          springBuffer(ctx),
          adjacencyBuffer(ctx),
          adjacencySpringBuffer(ctx),
          groupBuffer(ctx),
          indexBuffer(ctx),
          predictPushConstants(ctx),
          solvePushConstants(ctx),
          finalizePushConstants(ctx)
    {
        // PushConstants 의 생성자는 stageFlags 를 멤버에 넣지 않는다 (지역 변수에 넣는다).
        // 설정하지 않으면 쓰레기 값으로 vkCmdPushConstants 가 호출된다.
        this->predictPushConstants.setStageFlags(VK_SHADER_STAGE_COMPUTE_BIT);
        this->solvePushConstants.setStageFlags(VK_SHADER_STAGE_COMPUTE_BIT);
        this->finalizePushConstants.setStageFlags(VK_SHADER_STAGE_COMPUTE_BIT);
    }

    ClothDataManagement::~ClothDataManagement()
    {
        this->cleanup();
    }

    void ClothDataManagement::create(const ClothSimulation& cloth)
    {
        if (!cloth.isValid())
        {
            Log::exitToLogger("ClothDataManagement::create — ClothSimulation 데이터가 올바르지 않다");
        }

        this->ready = false;

        this->vertexCount = cloth.getVertexCount();
        this->indexCount = cloth.getIndexCount();
        this->iterations = cloth.getInfo().iterations;
        this->floorY = cloth.getInfo().floorY;
        this->groupRanges = cloth.getGroupRanges();

        this->createPipelines();
        this->createBuffers(cloth);
        this->createDescriptorSet();

        this->ready = true;

        Log::printToLogger("[ClothDataManagement] vertices %u / indices %u / groups %u / iterations %u",
                           this->vertexCount, this->indexCount, this->getGroupCount(), this->iterations);
        Log::printToLogger("  dispatch / frame : %u", this->getDispatchCount());
        Log::printToLogger("  barrier  / frame : %u", this->getBarrierCount());
    }

    void ClothDataManagement::createPipelines()
    {
        // 파이프라인은 격자 크기와 무관하므로 한 번만 만든다.
        if (this->pipelinesCreated)
        {
            return;
        }

        // PipelineConfig::name 이 ShaderManager 의 키와 같아야 한다.
        // PipelineHandle::createCommon() 이 그 이름으로
        // getLayoutsForPipeline() / getPushConstantRange() 를 찾는다.
        this->predictPipeline.createFromConfig(PipelineConfig::createClothPredict());
        this->solvePipeline.createFromConfig(PipelineConfig::createClothSolve());
        this->finalizePipeline.createFromConfig(PipelineConfig::createClothFinalize());

        this->pipelinesCreated = true;
    }

    void ClothDataManagement::createBuffers(const ClothSimulation& cloth)
    {
        this->cleanupBuffers();

        // createStorageBuffer() 의 두 번째 인자는 "추가" 가 아니라 usage 전체를 덮어쓴다
        // (BaseBuffer2.cpp 의 bufferInfo.usage = additionalUsage).
        // 그래서 STORAGE_BUFFER_BIT 를 직접 넣어야 한다.
        const VkBufferUsageFlags storageUsage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        // ---- binding 0 : simulationParams (UBO) --------------------------
        // nullptr 로 만들고 updateData() 로 채운다.
        // BaseBuffer2::create() 는 data 가 있으면 allocatedSize(정렬된 크기) 만큼
        // memcpy 하므로, 16 byte 구조체를 그대로 넘기면 구조체 밖을 읽는다.
        SimulationParams params = cloth.getSimulationParams();
        this->paramsBuffer.createUniformBuffer(sizeof(SimulationParams), nullptr);
        this->paramsBuffer.updateData(&params, sizeof(SimulationParams), 0);

        // ---- binding 1 : vertices ---------------------------------------
        // VERTEX_BUFFER_BIT : Renderer 의 "cloth" 패스가 이 버퍼를 그대로 정점 버퍼로 쓴다.
        //                     나중에 추가하려면 버퍼를 다시 만들어야 하므로 지금 넣는다.
        // TRANSFER_SRC_BIT  : downloadVertices() 로 결과를 CPU 에서 확인하기 위한 것.
        const VkDeviceSize vertexBufferSize = sizeof(ClothVertex) * cloth.getVertices().size();
        this->vertexBuffer.createStorageBuffer(vertexBufferSize,
                                               storageUsage |
                                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        this->vertexBuffer.copyData(cloth.getVertices().data(), vertexBufferSize);

        // ---- binding 2 : springs ----------------------------------------
        const VkDeviceSize springBufferSize = sizeof(Spring) * cloth.getSprings().size();
        this->springBuffer.createStorageBuffer(springBufferSize, storageUsage);
        this->springBuffer.copyData(cloth.getSprings().data(), springBufferSize);

        // ---- binding 3 : adjacency (CSR 의 offset + count) ---------------
        const VkDeviceSize adjacencyBufferSize =
            sizeof(ClothVertexAdjacency) * cloth.getAdjacency().size();
        this->adjacencyBuffer.createStorageBuffer(adjacencyBufferSize, storageUsage);
        this->adjacencyBuffer.copyData(cloth.getAdjacency().data(), adjacencyBufferSize);

        // ---- binding 4 : adjacencySprings (CSR 의 값) --------------------
        const VkDeviceSize adjacencySpringBufferSize =
            sizeof(cUint32_t) * cloth.getAdjacencySprings().size();
        this->adjacencySpringBuffer.createStorageBuffer(adjacencySpringBufferSize, storageUsage);
        this->adjacencySpringBuffer.copyData(cloth.getAdjacencySprings().data(),
                                            adjacencySpringBufferSize);

        // ---- binding 5 : groupVertices (그룹별로 정렬된 정점 번호) -------
        const VkDeviceSize groupBufferSize = sizeof(cUint32_t) * cloth.getGroupVertices().size();
        this->groupBuffer.createStorageBuffer(groupBufferSize, storageUsage);
        this->groupBuffer.copyData(cloth.getGroupVertices().data(), groupBufferSize);

        // ---- 인덱스 버퍼 (디스크립터가 아니다) ---------------------------
        // createIndexBuffer() 를 쓰면 안 된다. 그 함수는 HOST_VISIBLE 을 요청하지만
        // BaseBuffer2::create() 는 그 인자를 무시하고 DEVICE_LOCAL 을 먼저 고른다.
        // 그러면 copyData() 가 스테이징 경로로 가는데 TRANSFER_DST_BIT 가 없어서
        // vkCmdCopyBuffer 가 거부당하고 — 복사가 조용히 통째로 사라진다.
        // 증상은 "인덱스가 전부 0 이라 삼각형이 전부 퇴화 = 아무것도 안 그려짐" 이다.
        // createStorageBuffer() 의 두 번째 인자가 usage 전체를 정하므로 그것을 쓴다.
        const VkDeviceSize indexBufferSize = sizeof(cUint32_t) * cloth.getIndices().size();
        this->indexBuffer.createStorageBuffer(indexBufferSize,
                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        this->indexBuffer.copyData(cloth.getIndices().data(), indexBufferSize);

    }

    void ClothDataManagement::createDescriptorSet()
    {
        // 순서 = clothCommon.glsl 의 binding 번호.
        // DescriptorSetHandle::create() 가 벡터의 인덱스를 binding 번호로 쓴다.
        this->descriptorSet.create(this->ctx, {std::ref<ShaderResource>(this->paramsBuffer),
                                               std::ref<ShaderResource>(this->vertexBuffer),
                                               std::ref<ShaderResource>(this->springBuffer),
                                               std::ref<ShaderResource>(this->adjacencyBuffer),
                                               std::ref<ShaderResource>(this->adjacencySpringBuffer),
                                               std::ref<ShaderResource>(this->groupBuffer)});
    }

    void ClothDataManagement::cleanupBuffers()
    {
        // createStorageBuffer() 는 cleanup() 을 먼저 부르지 않는다.
        // 그래서 다시 만들기 전에 직접 지워야 한다 (안 하면 버퍼가 새어 나간다).
        this->paramsBuffer.cleanup();
        this->vertexBuffer.cleanup();
        this->springBuffer.cleanup();
        this->adjacencyBuffer.cleanup();
        this->adjacencySpringBuffer.cleanup();
        this->groupBuffer.cleanup();
        this->indexBuffer.cleanup();
    }

    void ClothDataManagement::cleanup()
    {
        this->ready = false;

        this->cleanupBuffers();

        if (this->pipelinesCreated)
        {
            this->predictPipeline.cleanup();
            this->solvePipeline.cleanup();
            this->finalizePipeline.cleanup();
            this->pipelinesCreated = false;
        }

        this->groupRanges.clear();
        this->vertexCount = 0;
        this->indexCount = 0;
    }

    cUint32_t ClothDataManagement::getGroupCountX(cUint32_t threadCount)
    {
        return (threadCount + kWorkGroupSize - 1u) / kWorkGroupSize;
    }

    cUint32_t ClothDataManagement::getDispatchCount() const
    {
        // predict 1 + solve (그룹수 x 반복) + finalize 1
        return 2u + this->getGroupCount() * this->iterations;
    }

    cUint32_t ClothDataManagement::getBarrierCount() const
    {
        // solve dispatch 앞마다 하나 + finalize 앞 하나 + 프레임 끝 하나
        return this->getGroupCount() * this->iterations + 2u;
    }

    // -----------------------------------------------------------------
    // 재빌드 없이 즉시 반영되는 값들.
    //
    // 버퍼를 다시 만들지 않으므로 waitIdle() 이 필요 없고, 따라서 ImGui
    // 슬라이더를 드래그하는 중에도 프레임이 끊기지 않는다.
    // -----------------------------------------------------------------
    void ClothDataManagement::setIterations(cUint32_t iterations)
    {
        // 0 이면 solve 가 한 번도 돌지 않아 천이 그냥 자유낙하한다.
        this->iterations = (iterations == 0u) ? 1u : iterations;
    }

    void ClothDataManagement::setFloorY(cFloat floorY)
    {
        // step() 이 푸시 상수로 커맨드 버퍼에 박아 넣는다.
        this->floorY = floorY;
    }

    void ClothDataManagement::setSimulationParams(const SimulationParams& params)
    {
        if (!this->ready)
        {
            return;
        }

        // 단서 : paramsBuffer 는 HOST_VISIBLE 단일 버퍼다. 아직 제출이 끝나지
        // 않은 프레임이 이 UBO 를 읽고 있을 수 있어 엄밀히는 경쟁 상태다.
        // 다만 내용이 gravity(vec3) + h(float) 16 byte 뿐이고 둘 다 연속적으로
        // 변하는 값이라, 최악이 "한 프레임이 옛 gravity 와 새 h 를 섞어 읽는다"
        // 정도여서 눈에 보이지 않는다.
        // 엄밀히 하려면 MAX_FRAMES_IN_FLIGHT 개를 만들고 현재 프레임 것만 써야 한다.
        this->paramsBuffer.updateData(&params, sizeof(SimulationParams), 0);
    }

    void ClothDataManagement::bindPipeline(VkCommandBuffer cmd, PipelineHandle& pipeline)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipeline());

        // 세 파이프라인의 set 0 레이아웃은 같지만 푸시 상수 범위가 다르다.
        // Vulkan 은 푸시 상수 범위까지 같아야 "호환" 으로 보기 때문에,
        // 파이프라인을 바꾸면 디스크립터 바인딩이 무효가 된다. 그래서 매번 다시 묶는다.
        const VkDescriptorSet sets[] = {this->descriptorSet.get()};
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipeline.getPipelineLayout(), 0, 1, sets, 0, nullptr);
    }

    void ClothDataManagement::insertSolveBarrier(VkCommandBuffer cmd)
    {
        // 앞 dispatch 가 vertices[] 에 쓴 것을 다음 dispatch 가 읽을 수 있게 한다.
        // 같은 그룹 안에서는 서로 독립이지만, 그룹과 그룹 사이에는 의존이 있다
        // (Gauss-Seidel — 이웃의 최신 위치를 읽는 것이 수렴의 근거다).
        BarrierHelperFunction::insertMemoryBarrier(cmd,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    }

    void ClothDataManagement::insertFrameEndBarrier(VkCommandBuffer cmd)
    {
        // 두 가지를 동시에 막는다.
        //  1. 다음 프레임의 clothPredict 가 이 프레임의 결과를 읽는 것
        //     (배리어의 dst 범위는 같은 큐에 나중에 제출되는 명령까지 미친다)
        //  2. 천을 정점 버퍼로 그릴 때 읽는 것
        BarrierHelperFunction::insertMemoryBarrier(cmd,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT |
                VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT |
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
            VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT |
                VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT);
    }

    void ClothDataManagement::step(VkCommandBuffer cmd)
    {
        if (!this->ready || this->vertexCount == 0u || this->groupRanges.empty())
        {
            return;
        }

        // ---- 1. 관성 예측 : y = x + h·v + h²·g ---------------------------
        this->bindPipeline(cmd, this->predictPipeline);
        this->predictPushConstants.getData().numVertices = this->vertexCount;
        this->predictPushConstants.push(cmd, this->predictPipeline.getPipelineLayout());
        vkCmdDispatch(cmd, getGroupCountX(this->vertexCount), 1u, 1u);

        // ---- 2. 블록 좌표 하강 : 그룹별 뉴턴 한 발짝 ---------------------
        this->bindPipeline(cmd, this->solvePipeline);

        for (cUint32_t iteration = 0; iteration < this->iterations; iteration++)
        {
            for (const ClothSimulation::GroupRange& range : this->groupRanges)
            {
                // predict 또는 앞 그룹의 쓰기를 기다린다
                this->insertSolveBarrier(cmd);

                this->solvePushConstants.getData().groupOffset = range.offset;
                this->solvePushConstants.getData().groupCount = range.count;
                this->solvePushConstants.push(cmd, this->solvePipeline.getPipelineLayout());

                vkCmdDispatch(cmd, getGroupCountX(range.count), 1u, 1u);
            }
        }

        // ---- 3. 속도 환산 : v = (x - x(t)) / h ---------------------------
        this->insertSolveBarrier(cmd);

        this->bindPipeline(cmd, this->finalizePipeline);
        this->finalizePushConstants.getData().numVertices = this->vertexCount;
        this->finalizePushConstants.getData().floorY = this->floorY;
        this->finalizePushConstants.push(cmd, this->finalizePipeline.getPipelineLayout());
        vkCmdDispatch(cmd, getGroupCountX(this->vertexCount), 1u, 1u);

        this->insertFrameEndBarrier(cmd);
    }

    void ClothDataManagement::downloadVertices(std::vector<ClothVertex>& outVertices)
    {
        outVertices.clear();

        if (!this->ready || this->vertexCount == 0u)
        {
            return;
        }

        const VkDeviceSize size = sizeof(ClothVertex) * this->vertexCount;

        // 정점 버퍼는 DEVICE_LOCAL 이므로 CPU 에서 직접 읽을 수 없다.
        // host visible 스테이징 버퍼로 복사한 뒤 읽는다.
        BaseBuffer2 staging(this->ctx);
        staging.createStagingBuffer(size, nullptr);

        // compute 를 기록한 큐와 같은 패밀리를 쓴다.
        // transfer 큐는 패밀리가 다를 수 있어서 소유권 이전이 필요해진다.
        CommandBufferHandle commandBuffer =
            this->ctx.createGraphicsCommandBufferHandle(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer.getCommandBuffer(),
                        this->vertexBuffer.getBuffer(), staging.getBuffer(), 1, &copyRegion);

        commandBuffer.submitAndWait();

        outVertices.resize(this->vertexCount);
        memcpy(outVertices.data(), staging.getMapped(), size);
    }
}
