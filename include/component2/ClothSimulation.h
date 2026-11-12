#ifndef INCLUDE_COMPONENT2_CLOTHSIMULATION_H_
#define INCLUDE_COMPONENT2_CLOTHSIMULATION_H_

#include "common.h"

#include <vector>

namespace vkengine
{
    // ---------------------------------------------------------------------
    // shader/clothCommon.glsl 의 구조체와 1:1 로 대응한다.
    // 크기가 어긋나면 셰이더가 엉뚱한 메모리를 읽는데, 증상이 "천이 이상하게
    // 움직인다" 로만 나타나서 추적이 어렵다. 그래서 static_assert 로 못박는다.
    // ---------------------------------------------------------------------

    // GLSL : clothVertex (std430, 64 byte)
    struct ClothVertex
    {
        cVec3 position;         cFloat mass;        // x      — 현재 후보 위치
        cVec3 prevPosition;     cFloat pinned;      // x(t)   — 프레임 시작 위치
        cVec3 nextPosition;     cFloat padding1;    // y      — 관성 예측 위치 (반복 중 고정)
        cVec3 velocity;         cFloat padding2;    // v
    };

    // GLSL : spring (std430, 16 byte)
    struct Spring
    {
        cUint32_t vertexA;
        cUint32_t vertexB;
        cFloat restLength;      // 자연 길이 L
        cFloat stiffness;       // 강성 k
    };

    // GLSL : clothVertexAdjacency (std430, 8 byte) — CSR 의 (시작 위치, 개수)
    struct ClothVertexAdjacency
    {
        cUint32_t offset;
        cUint32_t count;
    };

    // GLSL : simulationParams (std140 UBO, 16 byte)
    struct SimulationParams
    {
        cVec3 gravity;
        cFloat h;
    };

    static_assert(sizeof(ClothVertex) == 64, "ClothVertex 는 GLSL clothVertex 와 64 byte 로 일치해야 한다");
    static_assert(sizeof(Spring) == 16, "Spring 은 GLSL spring 과 16 byte 로 일치해야 한다");
    static_assert(sizeof(ClothVertexAdjacency) == 8, "ClothVertexAdjacency 는 GLSL 과 8 byte 로 일치해야 한다");
    static_assert(sizeof(SimulationParams) == 16, "SimulationParams 는 GLSL simulationParams 와 16 byte 로 일치해야 한다");

    // ---------------------------------------------------------------------
    // Vertex Block Descent 기반 천 시뮬레이션의 CPU 측 데이터 생성.
    //
    // 이 클래스는 Vulkan 을 전혀 쓰지 않는다. std::vector 로 여섯 개 버퍼의
    // 내용을 만들고, 그것이 올바른지 검증하는 것까지만 담당한다.
    // 버퍼 생성과 dispatch 는 ClothDataManagement 가 맡는다.
    // ---------------------------------------------------------------------
    class ClothSimulation
    {
    public:
        // clothSolve.comp 한 번의 dispatch 범위.
        // 푸시 상수 groupOffset / groupCount 와 그대로 대응한다.
        struct GroupRange
        {
            cUint32_t offset;
            cUint32_t count;
        };

        struct Info
        {
            cUint32_t gridX = 32;                           // 가로 정점 수
            cUint32_t gridY = 32;                           // 세로 정점 수
            cFloat spacing = 0.05f;                         // 정점 간격 (m)
            cFloat mass = 0.01f;                            // 정점 하나의 질량 (kg)

            cFloat kStretch = 5000.0f;                      // 늘어남 — 가로/세로
            cFloat kShear = 2000.0f;                        // 전단   — 대각선
            cFloat kBend = 50.0f;                           // 굽힘   — 두 칸 건너

            cBool useShear = true;                          // 끄면 그룹 수가 줄어든다
            cBool useBend = true;
            cBool pinTopCorners = true;                     // 윗변 양 끝을 집게로 고정

            cVec3 origin = cVec3(0.0f, 2.0f, 0.0f);         // 윗변 왼쪽 끝의 위치
            cVec3 gravity = cVec3(0.0f, -9.8f, 0.0f);
            cFloat timeStep = 1.0f / 60.0f;                 // h
            cUint32_t iterations = 10;                      // 반복 횟수 (품질 손잡이)
            cFloat floorY = 0.0f;                           // clothFinalize.comp 의 임시 바닥

            // 완전한 평면은 대칭이라 면 밖으로 접힐 동기가 없다.
            // 아주 작은 값을 주면 주름이 생기기 시작한다. 검증할 때는 0 으로 둔다.
            cFloat initialJitterZ = 0.0f;
        };

        ClothSimulation() = default;
        ~ClothSimulation() = default;

        void create(const Info& info);
        void cleanup();

        const Info& getInfo() const { return this->info; }

        const std::vector<ClothVertex>& getVertices() const { return this->vertices; }
        const std::vector<Spring>& getSprings() const { return this->springs; }
        const std::vector<ClothVertexAdjacency>& getAdjacency() const { return this->adjacency; }
        const std::vector<cUint32_t>& getAdjacencySprings() const { return this->adjacencySprings; }
        const std::vector<cUint32_t>& getGroupVertices() const { return this->groupVertices; }
        const std::vector<GroupRange>& getGroupRanges() const { return this->groupRanges; }
        const std::vector<cUint32_t>& getIndices() const { return this->indices; }

        SimulationParams getSimulationParams() const;

        cUint32_t getVertexCount() const;
        cUint32_t getSpringCount() const;
        cUint32_t getGroupCount() const;
        cUint32_t getIndexCount() const;

        // 정점 하나가 어느 그룹에 속하는지 (검증과 디버깅용)
        cUint32_t getGroupOfVertex(cUint32_t vertexIndex) const;

        // --- 검증 ------------------------------------------------------
        // 같은 그룹의 두 정점이 스프링으로 연결되어 있지 않은지.
        // 이것이 깨지면 천이 미세하게 떨리는데, 원인 찾기가 매우 어렵다.
        cBool isGroupingValid() const;

        // CSR 이 스프링 목록과 일치하는지 (개수 합, offset 연속성, 소유 관계)
        cBool isAdjacencyValid() const;

        // 위 둘 + 기본 크기 검사
        cBool isValid() const;

        void logSummary() const;

    private:
        Info info{};

        std::vector<ClothVertex> vertices;              // binding 1
        std::vector<Spring> springs;                    // binding 2
        std::vector<ClothVertexAdjacency> adjacency;    // binding 3
        std::vector<cUint32_t> adjacencySprings;        // binding 4
        std::vector<cUint32_t> groupVertices;           // binding 5
        std::vector<GroupRange> groupRanges;            // dispatch 범위 (CPU 전용)
        std::vector<cUint32_t> groupOfVertex;           // 검증용 역색인 (CPU 전용)
        std::vector<cUint32_t> indices;                 // 렌더링용 삼각형 인덱스

        void buildGrid();
        void buildSprings();
        void buildAdjacency();
        void buildGroups();
        void buildIndices();

        cUint32_t getVertexIndex(cUint32_t x, cUint32_t y) const;

        // 주어진 순서대로 그리디 색칠을 수행하고 사용한 색 수를 돌려준다.
        // 색칠 결과는 순서에 크게 좌우되므로 buildGroups() 가 여러 순서를 시도한다.
        cUint32_t colorVertices(const std::vector<cUint32_t>& order,
                                std::vector<cUint32_t>& outGroupOfVertex) const;

        // DSATUR — 매번 "이웃이 쓰는 색 종류가 가장 많은" 정점을 먼저 칠한다.
        // 순서를 미리 정하는 그리디보다 격자형 그래프에서 색 수가 적게 나온다.
        cUint32_t colorVerticesDsatur(std::vector<cUint32_t>& outGroupOfVertex) const;

        // 정점 i 에 스프링으로 직접 연결된 정점들
        void collectNeighbors(cUint32_t vertexIndex, std::vector<cUint32_t>& outNeighbors) const;

        // 자연 길이는 생성 당시의 실제 거리로 잡는다.
        // 그래야 "중력을 0 으로 두면 천이 움직이지 않는다" 가 항상 성립하고,
        // 그 성질을 데이터 검증 테스트로 쓸 수 있다.
        void addSpring(cUint32_t a, cUint32_t b, cFloat stiffness);
    };
}

#endif // !INCLUDE_COMPONENT2_CLOTHSIMULATION_H_
