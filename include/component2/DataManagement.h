#ifndef INCLUDE_COMPONENT2_DATAMANAGEMENT_H_
#define INCLUDE_COMPONENT2_DATAMANAGEMENT_H_

// CPU 에서 만든 시뮬레이션 데이터를 GPU 자원으로 올려 관리하는 클래스들이
// 모이는 파일이다. 지금은 ClothDataManagement 하나뿐이다.

#include "common.h"

#include "Context.h"
#include "ShaderManager.h"
#include "PipelineHandle.h"
#include "DescriptorSetHandle.h"
#include "BaseBuffer2.h"
#include "PushConstants.h"

#include "ClothSimulation.h"

#include <vector>

namespace vkengine
{
    // ---------------------------------------------------------------------
    // 천 시뮬레이션의 GPU 쪽 데이터 관리.
    //
    // 역할 구분
    //   ClothSimulation      CPU 데이터를 만들고 검증한다. Vulkan 을 전혀 모른다.
    //   ClothDataManagement  그 데이터를 GPU 버퍼로 올리고, 파이프라인과
    //                        디스크립터를 들고 있고, 매 프레임 compute 를 기록한다.
    //
    // 왜 나누는가
    //   ClothSimulation 이 Vulkan 을 모르기 때문에 GPU 없이 cl.exe 로 단독
    //   컴파일해서 그룹 분할과 CSR 을 검증할 수 있다. 그 성질을 지키려고
    //   Vulkan 자원은 전부 이 클래스 한 곳에 모아 둔다.
    //
    // 한 프레임의 순서 (Vertex Block Descent)
    //   clothPredict               1 회     y = x + h·v + h²·g
    //   clothSolve   그룹수 x 반복 회        그룹 안에서는 서로 독립 → 병렬
    //   clothFinalize              1 회     v = (x - x(t)) / h
    //
    // 배리어
    //   dispatch 사이마다 compute→compute 메모리 배리어가 필요하다.
    //   빠뜨려도 검증 레이어는 아무 말도 하지 않고, 증상은 "천이 미세하게 떨린다"
    //   로만 나타나서 원인을 찾기가 매우 어렵다.
    // ---------------------------------------------------------------------
    class ClothDataManagement
    {
    public:
        ClothDataManagement(Context& ctx, ShaderManager& shaderManager);
        ~ClothDataManagement();

        ClothDataManagement(const ClothDataManagement&) = delete;
        ClothDataManagement& operator=(const ClothDataManagement&) = delete;

        // 버퍼를 만들고 CPU 데이터를 올린다.
        // 다시 불러도 된다 (ImGui 의 Apply / Reset). 단, 호출 전에 GPU 가
        // 그 버퍼를 쓰고 있지 않아야 한다 — 부르는 쪽에서 waitIdle() 을 해야 한다.
        void create(const ClothSimulation& cloth);
        void cleanup();

        // 프레임 커맨드 버퍼에 한 스텝을 기록한다.
        // 그래픽 패스보다 먼저 기록해야 한다 - Renderer 의 "cloth" 패스가 이 결과를 읽는다.
        void step(VkCommandBuffer cmd);

        cBool isReady() const { return this->ready; }

        cUint32_t getVertexCount() const { return this->vertexCount; }
        cUint32_t getGroupCount() const { return static_cast<cUint32_t>(this->groupRanges.size()); }
        cUint32_t getIterations() const { return this->iterations; }
        cFloat getFloorY() const { return this->floorY; }

        // --- 재빌드 없이 즉시 반영되는 값 -------------------------------
        // 버퍼 크기도 스프링 구성도 건드리지 않으므로 create() 를 다시 부를
        // 필요가 없다. ImGui 슬라이더를 드래그하면서 결과를 바로 볼 수 있다.
        //
        // iterations / floorY 는 커맨드 버퍼를 '기록할 때' 읽히므로 GPU 와
        // 경쟁하지 않는다. setSimulationParams() 만 단서가 있다 (구현부 주석).
        void setIterations(cUint32_t iterations);
        void setFloorY(cFloat floorY);
        void setSimulationParams(const SimulationParams& params);

        // 한 프레임에 들어가는 vkCmdDispatch / 배리어 횟수 (ImGui 표시용)
        cUint32_t getDispatchCount() const;
        cUint32_t getBarrierCount() const;

        // 천을 그릴 때 쓰는 것들.
        // 정점 버퍼는 compute 가 고쳐 쓰는 그 버퍼다 (따로 복사하지 않는다).
        BaseBuffer2& getVertexBuffer() { return this->vertexBuffer; }
        BaseBuffer2& getIndexBuffer() { return this->indexBuffer; }
        cUint32_t getIndexCount() const { return this->indexCount; }

        // 디버그용 — GPU 의 정점을 CPU 로 가져온다.
        // 내부에서 큐를 기다리므로 프레임 루프 안에서 매 프레임 부르면 안 된다.
        void downloadVertices(std::vector<ClothVertex>& outVertices);

    private:
        // 셰이더의 layout(local_size_x = 256) 과 반드시 같아야 한다.
        static const cUint32_t kWorkGroupSize = 256u;

        // 셰이더의 layout(push_constant) 블록과 1:1 로 대응한다.
        struct PredictPushConstants
        {
            cUint32_t numVertices;
        };

        struct SolvePushConstants
        {
            cUint32_t groupOffset;
            cUint32_t groupCount;
        };

        struct FinalizePushConstants
        {
            cUint32_t numVertices;
            cFloat floorY;
        };

        // 셰이더와 어긋나면 dispatch 범위가 엉뚱한 값으로 정해진다.
        // 증상은 "천의 일부만 움직인다" 로 나타나므로 여기서 못박는다.
        static_assert(sizeof(PredictPushConstants) == 4,
                      "clothPredict.comp 의 push_constant 는 uint 하나다");
        static_assert(sizeof(SolvePushConstants) == 8,
                      "clothSolve.comp 의 push_constant 는 uint 둘이다");
        static_assert(sizeof(FinalizePushConstants) == 8,
                      "clothFinalize.comp 의 push_constant 는 uint + float 이다");

        void createPipelines();
        void createBuffers(const ClothSimulation& cloth);
        void createDescriptorSet();
        void cleanupBuffers();

        void bindPipeline(VkCommandBuffer cmd, PipelineHandle& pipeline);

        // dispatch 사이의 compute→compute 배리어
        void insertSolveBarrier(VkCommandBuffer cmd);

        // 마지막 dispatch 뒤 — 다음 프레임의 predict 와 천 그리기를 위한 것
        void insertFrameEndBarrier(VkCommandBuffer cmd);

        static cUint32_t getGroupCountX(cUint32_t threadCount);

        Context& ctx;
        ShaderManager& shaderManager;

        PipelineHandle predictPipeline;
        PipelineHandle solvePipeline;
        PipelineHandle finalizePipeline;
        cBool pipelinesCreated = false;

        // clothCommon.glsl 의 set 0 바인딩 순서와 같아야 한다.
        BaseBuffer2 paramsBuffer;           // binding 0 — simulationParams (UBO)
        BaseBuffer2 vertexBuffer;           // binding 1 — vertices[]
        BaseBuffer2 springBuffer;           // binding 2 — springs[]
        BaseBuffer2 adjacencyBuffer;        // binding 3 — adjacency[]
        BaseBuffer2 adjacencySpringBuffer;  // binding 4 — adjacencySprings[]
        BaseBuffer2 groupBuffer;            // binding 5 — groupVertices[]

        // 디스크립터가 아니다 — vkCmdBindIndexBuffer 로만 쓴다.
        // 격자는 모양이 바뀌지 않으므로 한 번 올리고 그대로 둔다.
        BaseBuffer2 indexBuffer;

        // 세 셰이더가 clothCommon.glsl 하나를 공유하므로 set 0 의 구성이 완전히 같다.
        // 그래서 디스크립터 세트는 하나만 만들어 세 파이프라인에 그대로 쓴다.
        DescriptorSetHandle descriptorSet;

        PushConstants<PredictPushConstants> predictPushConstants;
        PushConstants<SolvePushConstants> solvePushConstants;
        PushConstants<FinalizePushConstants> finalizePushConstants;

        std::vector<ClothSimulation::GroupRange> groupRanges;

        cUint32_t vertexCount = 0;
        cUint32_t indexCount = 0;
        cUint32_t iterations = 0;
        cFloat floorY = 0.0f;
        cBool ready = false;
    };
}

#endif // !INCLUDE_COMPONENT2_DATAMANAGEMENT_H_
