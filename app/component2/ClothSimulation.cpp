#include "ClothSimulation.h"

#include "macros.h"

#include <algorithm>
#include <cassert>

namespace vkengine
{
    namespace
    {
        // 아직 그룹이 정해지지 않은 정점을 표시하는 값
        const cUint32_t kInvalidGroup = 0xFFFFFFFFu;
    }

    void ClothSimulation::create(const Info& info)
    {
        this->cleanup();
        this->info = info;

        assert(this->info.gridX >= 2 && this->info.gridY >= 2);

        this->buildGrid();
        this->buildSprings();
        this->buildAdjacency();
        this->buildGroups();
        this->buildIndices();

        _CHECK_RESULT_(this->isValid());
    }

    void ClothSimulation::cleanup()
    {
        this->vertices.clear();
        this->springs.clear();
        this->adjacency.clear();
        this->adjacencySprings.clear();
        this->groupVertices.clear();
        this->groupRanges.clear();
        this->groupOfVertex.clear();
        this->indices.clear();
    }

    // -----------------------------------------------------------------
    // 1. 격자 정점 생성
    //
    //   x 는 오른쪽, y 는 아래쪽으로 자란다. y = 0 이 천의 윗변이다.
    //   윗변 양 끝 두 개를 집게로 고정한다.
    // -----------------------------------------------------------------
    void ClothSimulation::buildGrid()
    {
        const cUint32_t vertexCount = this->info.gridX * this->info.gridY;

        this->vertices.clear();
        this->vertices.reserve(vertexCount);

        for (cUint32_t y = 0; y < this->info.gridY; y++)
        {
            for (cUint32_t x = 0; x < this->info.gridX; x++)
            {
                // 체커보드 모양으로 번갈아 밀어서 평면 대칭을 깬다 (기본값 0 이면 무효)
                const cFloat jitterSign = (((x + y) % 2u) == 0u) ? 1.0f : -1.0f;
                const cFloat jitterZ = this->info.initialJitterZ * jitterSign;

                const cVec3 position = this->info.origin
                    + cVec3(static_cast<cFloat>(x) * this->info.spacing,
                           -static_cast<cFloat>(y) * this->info.spacing,
                            jitterZ);

                ClothVertex vertex{};
                vertex.position = position;
                vertex.mass = this->info.mass;
                vertex.prevPosition = position;
                vertex.pinned = 0.0f;
                vertex.nextPosition = position;
                vertex.padding1 = 0.0f;
                vertex.velocity = cVec3(0.0f);
                vertex.padding2 = 0.0f;

                this->vertices.push_back(vertex);
            }
        }

        if (this->info.pinTopCorners)
        {
            this->vertices[this->getVertexIndex(0u, 0u)].pinned = 1.0f;
            this->vertices[this->getVertexIndex(this->info.gridX - 1u, 0u)].pinned = 1.0f;
        }
    }

    // -----------------------------------------------------------------
    // 2. 스프링 생성
    //
    //   늘어남 : 가로/세로 이웃      — 천이 늘어나지 않게 한다 (가장 중요)
    //   전단   : 대각선              — 천이 평행사변형으로 찌그러지지 않게 한다
    //   굽힘   : 두 칸 건너          — 주름의 크기를 결정한다
    //
    //   전단과 굽힘을 켤수록 한 정점이 더 많은 정점과 연결되므로 그룹 수가 늘고,
    //   dispatch 횟수도 늘어난다.
    // -----------------------------------------------------------------
    void ClothSimulation::buildSprings()
    {
        const cUint32_t nx = this->info.gridX;
        const cUint32_t ny = this->info.gridY;

        this->springs.clear();

        for (cUint32_t y = 0; y < ny; y++)
        {
            for (cUint32_t x = 0; x < nx; x++)
            {
                // 늘어남 — 가로, 세로
                if (x + 1u < nx)
                {
                    this->addSpring(this->getVertexIndex(x, y), this->getVertexIndex(x + 1u, y), this->info.kStretch);
                }
                if (y + 1u < ny)
                {
                    this->addSpring(this->getVertexIndex(x, y), this->getVertexIndex(x, y + 1u), this->info.kStretch);
                }

                // 전단 — 셀 하나의 두 대각선
                if (this->info.useShear && (x + 1u < nx) && (y + 1u < ny))
                {
                    this->addSpring(this->getVertexIndex(x, y), this->getVertexIndex(x + 1u, y + 1u), this->info.kShear);
                    this->addSpring(this->getVertexIndex(x + 1u, y), this->getVertexIndex(x, y + 1u), this->info.kShear);
                }

                // 굽힘 — 두 칸 건너
                if (this->info.useBend)
                {
                    if (x + 2u < nx)
                    {
                        this->addSpring(this->getVertexIndex(x, y), this->getVertexIndex(x + 2u, y), this->info.kBend);
                    }
                    if (y + 2u < ny)
                    {
                        this->addSpring(this->getVertexIndex(x, y), this->getVertexIndex(x, y + 2u), this->info.kBend);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------
    // 3. CSR 변환
    //
    //   스프링 목록은 "스프링 -> 정점 두 개" 방향이다.
    //   셰이더는 "정점 -> 거기 붙은 스프링들" 방향이 필요하다. 그 역색인을
    //   offset + count 형태(CSR)로 만든다.
    //
    //   adjacencySprings = [ 정점0의 스프링들 | 정점1의 스프링들 | ... ]
    //   adjacency[i]     = { 그 구간의 시작 위치, 개수 }
    // -----------------------------------------------------------------
    void ClothSimulation::buildAdjacency()
    {
        const cUint32_t vertexCount = static_cast<cUint32_t>(this->vertices.size());

        this->adjacency.assign(vertexCount, ClothVertexAdjacency{ 0u, 0u });

        // 1단계 — 정점별 개수 세기
        for (const Spring& spring : this->springs)
        {
            this->adjacency[spring.vertexA].count++;
            this->adjacency[spring.vertexB].count++;
        }

        // 2단계 — 누적합으로 시작 위치 계산
        cUint32_t running = 0u;
        for (cUint32_t i = 0; i < vertexCount; i++)
        {
            this->adjacency[i].offset = running;
            running += this->adjacency[i].count;
        }

        // 3단계 — 채우기. count 를 커서로 쓰면 값이 망가지므로 별도 커서를 둔다
        this->adjacencySprings.assign(running, 0u);

        std::vector<cUint32_t> cursor(vertexCount, 0u);

        for (cUint32_t j = 0; j < static_cast<cUint32_t>(this->springs.size()); j++)
        {
            const Spring& spring = this->springs[j];

            this->adjacencySprings[this->adjacency[spring.vertexA].offset + cursor[spring.vertexA]] = j;
            cursor[spring.vertexA]++;

            this->adjacencySprings[this->adjacency[spring.vertexB].offset + cursor[spring.vertexB]] = j;
            cursor[spring.vertexB]++;
        }
    }

    // -----------------------------------------------------------------
    // 4. 그룹 분할 (그리디 색칠)
    //
    //   스프링으로 직접 연결된 두 정점은 같은 그룹에 들어가면 안 된다.
    //   같은 그룹은 한 번의 dispatch 로 동시에 갱신되는데, 이웃끼리 동시에
    //   움직이면 "나머지는 고정" 가정이 깨져서 천이 떨린다.
    //
    //   그룹 수가 적을수록 dispatch 가 줄어 빠르다. 그리디는 최소를 보장하지
    //   않지만 격자에서는 거의 최적(2~4개)이 나온다.
    // -----------------------------------------------------------------
    cUint32_t ClothSimulation::colorVertices(const std::vector<cUint32_t>& order,
                                             std::vector<cUint32_t>& outGroupOfVertex) const
    {
        const cUint32_t vertexCount = static_cast<cUint32_t>(this->vertices.size());

        outGroupOfVertex.assign(vertexCount, kInvalidGroup);

        // blockedBy[g] == i 이면 "정점 i 를 처리하는 중에 색 g 가 이웃에게 점유됨" 이라는 뜻.
        // 초기값이 kInvalidGroup 이고 정점 번호와 절대 겹치지 않으므로 매번 비울 필요가 없다.
        std::vector<cUint32_t> blockedBy;

        for (const cUint32_t i : order)
        {
            const ClothVertexAdjacency entry = this->adjacency[i];

            // 이웃이 이미 쓰고 있는 색을 표시한다
            for (cUint32_t n = 0; n < entry.count; n++)
            {
                const Spring& spring = this->springs[this->adjacencySprings[entry.offset + n]];
                const cUint32_t other = (spring.vertexA == i) ? spring.vertexB : spring.vertexA;
                const cUint32_t otherGroup = outGroupOfVertex[other];

                if (otherGroup == kInvalidGroup)
                {
                    continue;
                }

                if (otherGroup >= static_cast<cUint32_t>(blockedBy.size()))
                {
                    blockedBy.resize(otherGroup + 1u, kInvalidGroup);
                }
                blockedBy[otherGroup] = i;
            }

            // 막히지 않은 가장 작은 색을 고른다
            cUint32_t chosen = kInvalidGroup;
            for (cUint32_t g = 0; g < static_cast<cUint32_t>(blockedBy.size()); g++)
            {
                if (blockedBy[g] != i)
                {
                    chosen = g;
                    break;
                }
            }

            // 모든 색이 막혔으면 새 색을 추가한다
            if (chosen == kInvalidGroup)
            {
                chosen = static_cast<cUint32_t>(blockedBy.size());
                blockedBy.push_back(kInvalidGroup);
            }

            outGroupOfVertex[i] = chosen;
        }

        // 실제로 쓰인 색의 개수
        cUint32_t used = 0u;
        for (const cUint32_t group : outGroupOfVertex)
        {
            used = std::max(used, group + 1u);
        }

        return used;
    }

    void ClothSimulation::collectNeighbors(cUint32_t vertexIndex, std::vector<cUint32_t>& outNeighbors) const
    {
        const ClothVertexAdjacency entry = this->adjacency[vertexIndex];

        outNeighbors.clear();
        outNeighbors.reserve(entry.count);

        for (cUint32_t n = 0; n < entry.count; n++)
        {
            const Spring& spring = this->springs[this->adjacencySprings[entry.offset + n]];
            outNeighbors.push_back((spring.vertexA == vertexIndex) ? spring.vertexB : spring.vertexA);
        }
    }

    cUint32_t ClothSimulation::colorVerticesDsatur(std::vector<cUint32_t>& outGroupOfVertex) const
    {
        const cUint32_t vertexCount = static_cast<cUint32_t>(this->vertices.size());

        outGroupOfVertex.assign(vertexCount, kInvalidGroup);

        // 이웃 목록을 한 번만 펼쳐 둔다
        std::vector<std::vector<cUint32_t>> neighborLists(vertexCount);
        for (cUint32_t i = 0; i < vertexCount; i++)
        {
            this->collectNeighbors(i, neighborLists[i]);
        }

        // saturation[i] = 정점 i 의 이웃이 쓰고 있는 "서로 다른" 색의 개수.
        // 이웃이 칠해질 때마다 증분으로 갱신한다. 매 단계 다시 세면 O(n² · 차수) 가
        // 되어 격자를 키울 때 재생성이 눈에 띄게 느려진다.
        std::vector<cUint32_t> saturation(vertexCount, 0u);
        std::vector<std::vector<cBool>> colorUsed(vertexCount);

        cUint32_t used = 0u;

        for (cUint32_t step = 0; step < vertexCount; step++)
        {
            // 1) 포화도가 가장 큰 미채색 정점을 고른다.
            //    동점이면 차수가 큰 쪽, 그다음 번호가 작은 쪽 — 결과를 재현 가능하게 한다.
            cUint32_t picked = kInvalidGroup;
            cUint32_t pickedSaturation = 0u;
            cUint32_t pickedDegree = 0u;

            for (cUint32_t i = 0; i < vertexCount; i++)
            {
                if (outGroupOfVertex[i] != kInvalidGroup)
                {
                    continue;
                }

                const cUint32_t degree = static_cast<cUint32_t>(neighborLists[i].size());

                if (picked == kInvalidGroup
                    || saturation[i] > pickedSaturation
                    || (saturation[i] == pickedSaturation && degree > pickedDegree))
                {
                    picked = i;
                    pickedSaturation = saturation[i];
                    pickedDegree = degree;
                }
            }

            // 2) 이웃이 쓰지 않는 가장 작은 색을 준다
            const std::vector<cBool>& blocked = colorUsed[picked];

            cUint32_t chosen = used;
            for (cUint32_t g = 0; g < used; g++)
            {
                if (g >= static_cast<cUint32_t>(blocked.size()) || !blocked[g])
                {
                    chosen = g;
                    break;
                }
            }

            outGroupOfVertex[picked] = chosen;
            used = std::max(used, chosen + 1u);

            // 3) 이웃들의 포화도를 갱신한다
            for (const cUint32_t other : neighborLists[picked])
            {
                if (outGroupOfVertex[other] != kInvalidGroup)
                {
                    continue;
                }

                std::vector<cBool>& otherUsed = colorUsed[other];

                if (chosen >= static_cast<cUint32_t>(otherUsed.size()))
                {
                    otherUsed.resize(chosen + 1u, false);
                }

                if (!otherUsed[chosen])
                {
                    otherUsed[chosen] = true;
                    saturation[other]++;
                }
            }
        }

        return used;
    }

    void ClothSimulation::buildGroups()
    {
        const cUint32_t vertexCount = static_cast<cUint32_t>(this->vertices.size());

        // 1차 — 정점 번호 순서로 그리디
        std::vector<cUint32_t> order(vertexCount);
        for (cUint32_t i = 0; i < vertexCount; i++)
        {
            order[i] = i;
        }

        cUint32_t groupCount = this->colorVertices(order, this->groupOfVertex);

        // 2차 이후 — iterated greedy.
        // 직전 결과의 색 묶음을 "큰 색 번호부터" 재배열해 다시 색칠한다.
        // 이 방법은 색 수를 늘리지 않으면서 자주 줄여 준다.
        // 그룹 수는 dispatch 횟수에 직접 곱해지므로 여기서 줄이는 게 그대로 성능이다.
        std::vector<cUint32_t> candidate;
        const cUint32_t kMaxPasses = 32u;

        for (cUint32_t pass = 0; pass < kMaxPasses; pass++)
        {
            std::vector<std::vector<cUint32_t>> buckets(groupCount);
            for (cUint32_t i = 0; i < vertexCount; i++)
            {
                buckets[this->groupOfVertex[i]].push_back(i);
            }

            order.clear();
            for (cUint32_t g = groupCount; g > 0u; g--)
            {
                const std::vector<cUint32_t>& bucket = buckets[g - 1u];
                order.insert(order.end(), bucket.begin(), bucket.end());
            }

            const cUint32_t candidateCount = this->colorVertices(order, candidate);

            if (candidateCount >= groupCount)
            {
                break; // 더 줄지 않으면 중단
            }

            this->groupOfVertex = candidate;
            groupCount = candidateCount;
        }

        // 3차 — DSATUR 로 한 번 더 시도하고, 더 좋으면 바꾼다
        const cUint32_t dsaturCount = this->colorVerticesDsatur(candidate);

        if (dsaturCount < groupCount)
        {
            this->groupOfVertex = candidate;
            groupCount = dsaturCount;
        }

        std::vector<cUint32_t> counts(groupCount, 0u);
        for (cUint32_t i = 0; i < vertexCount; i++)
        {
            counts[this->groupOfVertex[i]]++;
        }

        this->groupRanges.assign(groupCount, GroupRange{ 0u, 0u });

        cUint32_t running = 0u;
        for (cUint32_t g = 0; g < groupCount; g++)
        {
            this->groupRanges[g].offset = running;
            this->groupRanges[g].count = counts[g];
            running += counts[g];
        }

        // 같은 그룹끼리 모아서 정점 번호를 나열한다 (셰이더의 groupVertices)
        this->groupVertices.assign(vertexCount, 0u);

        std::vector<cUint32_t> cursor(groupCount, 0u);
        for (cUint32_t i = 0; i < vertexCount; i++)
        {
            const cUint32_t g = this->groupOfVertex[i];

            this->groupVertices[this->groupRanges[g].offset + cursor[g]] = i;
            cursor[g]++;
        }
    }

    // -----------------------------------------------------------------
    // 5. 렌더링용 삼각형 인덱스
    //
    //   셀 하나당 삼각형 두 개. 위치 버퍼를 그대로 vertex buffer 로 쓰므로
    //   인덱스만 있으면 렌더링이 된다.
    // -----------------------------------------------------------------
    void ClothSimulation::buildIndices()
    {
        const cUint32_t nx = this->info.gridX;
        const cUint32_t ny = this->info.gridY;

        this->indices.clear();
        this->indices.reserve(static_cast<cSize>(nx - 1u) * (ny - 1u) * 6u);

        for (cUint32_t y = 0; y + 1u < ny; y++)
        {
            for (cUint32_t x = 0; x + 1u < nx; x++)
            {
                const cUint32_t topLeft = this->getVertexIndex(x, y);
                const cUint32_t topRight = this->getVertexIndex(x + 1u, y);
                const cUint32_t bottomLeft = this->getVertexIndex(x, y + 1u);
                const cUint32_t bottomRight = this->getVertexIndex(x + 1u, y + 1u);

                // y 가 아래쪽(-Y)으로 자라므로, 화면에서 뒤집혀 보이면
                // 파이프라인의 cullMode 를 끄거나 이 순서를 바꾼다.
                this->indices.push_back(topLeft);
                this->indices.push_back(bottomLeft);
                this->indices.push_back(topRight);

                this->indices.push_back(topRight);
                this->indices.push_back(bottomLeft);
                this->indices.push_back(bottomRight);
            }
        }
    }

    cUint32_t ClothSimulation::getVertexIndex(cUint32_t x, cUint32_t y) const
    {
        return y * this->info.gridX + x;
    }

    void ClothSimulation::addSpring(cUint32_t a, cUint32_t b, cFloat stiffness)
    {
        Spring spring{};
        spring.vertexA = a;
        spring.vertexB = b;
        spring.restLength = glm::distance(this->vertices[a].position, this->vertices[b].position);
        spring.stiffness = stiffness;

        this->springs.push_back(spring);
    }

    SimulationParams ClothSimulation::getSimulationParams() const
    {
        SimulationParams params{};
        params.gravity = this->info.gravity;
        params.h = this->info.timeStep;

        return params;
    }

    cUint32_t ClothSimulation::getVertexCount() const
    {
        return static_cast<cUint32_t>(this->vertices.size());
    }

    cUint32_t ClothSimulation::getSpringCount() const
    {
        return static_cast<cUint32_t>(this->springs.size());
    }

    cUint32_t ClothSimulation::getGroupCount() const
    {
        return static_cast<cUint32_t>(this->groupRanges.size());
    }

    cUint32_t ClothSimulation::getIndexCount() const
    {
        return static_cast<cUint32_t>(this->indices.size());
    }

    cUint32_t ClothSimulation::getGroupOfVertex(cUint32_t vertexIndex) const
    {
        if (vertexIndex >= static_cast<cUint32_t>(this->groupOfVertex.size()))
        {
            return kInvalidGroup;
        }

        return this->groupOfVertex[vertexIndex];
    }

    // -----------------------------------------------------------------
    // 검증 — 같은 그룹 안에 연결된 정점이 없어야 한다
    // -----------------------------------------------------------------
    cBool ClothSimulation::isGroupingValid() const
    {
        if (this->groupOfVertex.size() != this->vertices.size())
        {
            return false;
        }

        for (const cUint32_t group : this->groupOfVertex)
        {
            if (group == kInvalidGroup)
            {
                return false;
            }
        }

        for (const Spring& spring : this->springs)
        {
            if (this->groupOfVertex[spring.vertexA] == this->groupOfVertex[spring.vertexB])
            {
                return false;
            }
        }

        // groupVertices 는 모든 정점을 정확히 한 번씩 담아야 한다
        if (this->groupVertices.size() != this->vertices.size())
        {
            return false;
        }

        std::vector<cBool> seen(this->vertices.size(), false);
        for (const cUint32_t vertexIndex : this->groupVertices)
        {
            if (vertexIndex >= static_cast<cUint32_t>(seen.size()) || seen[vertexIndex])
            {
                return false;
            }
            seen[vertexIndex] = true;
        }

        // 각 그룹 구간 안의 정점은 모두 그 그룹이어야 한다
        for (cUint32_t g = 0; g < static_cast<cUint32_t>(this->groupRanges.size()); g++)
        {
            const GroupRange range = this->groupRanges[g];

            if (static_cast<cSize>(range.offset) + range.count > this->groupVertices.size())
            {
                return false;
            }

            for (cUint32_t n = 0; n < range.count; n++)
            {
                if (this->groupOfVertex[this->groupVertices[range.offset + n]] != g)
                {
                    return false;
                }
            }
        }

        return true;
    }

    // -----------------------------------------------------------------
    // 검증 — CSR 이 스프링 목록과 일치해야 한다
    // -----------------------------------------------------------------
    cBool ClothSimulation::isAdjacencyValid() const
    {
        if (this->adjacency.size() != this->vertices.size())
        {
            return false;
        }

        // 개수 합 == 스프링 수 x 2 (스프링 하나는 정점 두 개에 등록된다)
        cSize total = 0;
        for (const ClothVertexAdjacency& entry : this->adjacency)
        {
            total += entry.count;
        }

        if (total != this->springs.size() * 2u)
        {
            return false;
        }

        if (this->adjacencySprings.size() != total)
        {
            return false;
        }

        // offset 이 빈틈 없이 이어져야 한다
        cUint32_t expected = 0u;
        for (const ClothVertexAdjacency& entry : this->adjacency)
        {
            if (entry.offset != expected)
            {
                return false;
            }
            expected += entry.count;
        }

        // 각 항목이 가리키는 스프링이 실제로 그 정점에 붙어 있어야 한다
        for (cUint32_t i = 0; i < static_cast<cUint32_t>(this->adjacency.size()); i++)
        {
            const ClothVertexAdjacency entry = this->adjacency[i];

            for (cUint32_t n = 0; n < entry.count; n++)
            {
                const cUint32_t springIndex = this->adjacencySprings[entry.offset + n];

                if (springIndex >= static_cast<cUint32_t>(this->springs.size()))
                {
                    return false;
                }

                const Spring& spring = this->springs[springIndex];
                if (spring.vertexA != i && spring.vertexB != i)
                {
                    return false;
                }
            }
        }

        return true;
    }

    cBool ClothSimulation::isValid() const
    {
        if (this->vertices.empty() || this->springs.empty())
        {
            return false;
        }

        if (this->vertices.size() != static_cast<cSize>(this->info.gridX) * this->info.gridY)
        {
            return false;
        }

        // 스프링 끝점이 범위 안에 있어야 한다
        const cUint32_t vertexCount = static_cast<cUint32_t>(this->vertices.size());
        for (const Spring& spring : this->springs)
        {
            if (spring.vertexA >= vertexCount || spring.vertexB >= vertexCount)
            {
                return false;
            }
            if (spring.vertexA == spring.vertexB)
            {
                return false;
            }
            if (spring.restLength <= 0.0f || spring.stiffness <= 0.0f)
            {
                return false;
            }
        }

        if (!this->isAdjacencyValid())
        {
            return false;
        }

        if (!this->isGroupingValid())
        {
            return false;
        }

        return true;
    }

    void ClothSimulation::logSummary() const
    {
        cUint32_t minCount = 0xFFFFFFFFu;
        cUint32_t maxCount = 0u;

        for (const ClothVertexAdjacency& entry : this->adjacency)
        {
            minCount = std::min(minCount, entry.count);
            maxCount = std::max(maxCount, entry.count);
        }

        if (this->adjacency.empty())
        {
            minCount = 0u;
        }

        _PRINT_TO_CONSOLE_("[ClothSimulation] grid %ux%u\n", this->info.gridX, this->info.gridY);
        _PRINT_TO_CONSOLE_("  vertices        : %u\n", this->getVertexCount());
        _PRINT_TO_CONSOLE_("  springs         : %u (shear %s, bend %s)\n",
            this->getSpringCount(),
            this->info.useShear ? "on" : "off",
            this->info.useBend ? "on" : "off");
        _PRINT_TO_CONSOLE_("  adjacency total : %u\n", static_cast<cUint32_t>(this->adjacencySprings.size()));
        _PRINT_TO_CONSOLE_("  adjacency count : min %u, max %u\n", minCount, maxCount);
        _PRINT_TO_CONSOLE_("  indices         : %u\n", this->getIndexCount());
        _PRINT_TO_CONSOLE_("  groups          : %u\n", this->getGroupCount());

        for (cUint32_t g = 0; g < static_cast<cUint32_t>(this->groupRanges.size()); g++)
        {
            _PRINT_TO_CONSOLE_("    group %u : offset %u, count %u\n",
                g, this->groupRanges[g].offset, this->groupRanges[g].count);
        }

        _PRINT_TO_CONSOLE_("  dispatch / frame: %u (predict 1 + solve %u x %u + finalize 1)\n",
            2u + this->getGroupCount() * this->info.iterations,
            this->getGroupCount(),
            this->info.iterations);

        _PRINT_TO_CONSOLE_("  adjacency valid : %s\n", this->isAdjacencyValid() ? "true" : "false");
        _PRINT_TO_CONSOLE_("  grouping valid  : %s\n", this->isGroupingValid() ? "true" : "false");
    }
}
