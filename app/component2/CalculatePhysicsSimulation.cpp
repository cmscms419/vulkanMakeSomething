#include "CalculatePhysicsSimulation.h"
#include "random.h"

#include <algorithm>
#include <cmath>

namespace vkengine
{
    CalculatePhysicsSimulation::CalculatePhysicsSimulation(const Config &cfg)
        : config(cfg)
    {
        cellSize = config.objectRadius * 1.5f;

        positions.resize(config.objectCount);
        velocities.resize(config.objectCount);
        radii.assign(config.objectCount, config.objectRadius);
        invisibleFramesLeft.assign(config.objectCount, 0);
        collisionCooldown.assign(config.objectCount, 0);

        for (cUint32_t i = 0; i < config.objectCount; ++i)
        {
            respawn(i);
            invisibleFramesLeft[i] = 0; // 시작할 때는 바로 보이게
        }
    }

    CalculatePhysicsSimulation::~CalculatePhysicsSimulation()
    {
    }

    void CalculatePhysicsSimulation::update(cFloat deltaTime)
    {
        integrate(deltaTime);
        resolveWorldBoundary();
        rebuildGrid();       // 4-b: 이번 프레임 위치 기준으로 공간 그리드 재구성
        resolveCollisions(); // 4-b: 그리드 인접 셀만 검사 (기존 O(n^2) 전수비교 대체)
    }

    void CalculatePhysicsSimulation::integrate(cFloat dt)
    {
        for (cUint32_t i = 0; i < config.objectCount; ++i)
        {
            if (collisionCooldown[i] > 0)
                --collisionCooldown[i];

            if (invisibleFramesLeft[i] > 0)
            {
                --invisibleFramesLeft[i];
                continue;
            }
            positions[i] += velocities[i] * dt;
        }
    }

    void CalculatePhysicsSimulation::resolveWorldBoundary()
    {
        for (cUint32_t i = 0; i < config.objectCount; ++i)
        {
            if (invisibleFramesLeft[i] > 0)
                continue;

            cFloat r = radii[i];
            for (int axis = 0; axis < 3; ++axis)
            {
                if (positions[i][axis] - r < config.worldBounds.min[axis])
                {
                    positions[i][axis] = config.worldBounds.min[axis] + r;
                    velocities[i][axis] = std::abs(velocities[i][axis]);
                }
                if (positions[i][axis] + r > config.worldBounds.max[axis])
                {
                    positions[i][axis] = config.worldBounds.max[axis] - r;
                    velocities[i][axis] = -std::abs(velocities[i][axis]);
                }
            }
        }
    }

    void CalculatePhysicsSimulation::rebuildGrid()
    {
        grid.clear();

        for (cUint32_t i = 0; i < config.objectCount; ++i)
        {
            if (invisibleFramesLeft[i] > 0)
                continue; // 재생성 대기 중인(안 보이는) 객체는 충돌 후보가 될 필요 없음

            cIvec3 cell = worldToCell(positions[i]);
            grid[cellKey(cell)].push_back(i);
        }
    }

    void CalculatePhysicsSimulation::resolveCollisions()
    {
        for (cUint32_t i = 0; i < config.objectCount; ++i)
        {
            if (invisibleFramesLeft[i] > 0 || collisionCooldown[i] > 0)
                continue;

            Sphere a = Sphere(positions[i], radii[i]);
            cIvec3 cellOfI = worldToCell(positions[i]);

            cBool iConsumed = false;
            for (int dz = -1; dz <= 1 && !iConsumed; ++dz)
            {
                for (int dy = -1; dy <= 1 && !iConsumed; ++dy)
                {
                    for (int dx = -1; dx <= 1 && !iConsumed; ++dx)
                    {
                        auto it = grid.find(cellKey(cellOfI + cIvec3(dx, dy, dz)));
                        if (it == grid.end())
                            continue;

                        for (cUint32_t j : it->second)
                        {
                            if (j <= i)
                                continue; // i,j 쌍 중복 처리 방지 (어느 셀에서 발견되든 인덱스가 큰 쪽만 처리)
                            if (invisibleFramesLeft[j] > 0 || collisionCooldown[j] > 0)
                                continue;

                            Sphere b = Sphere(positions[j], radii[j]);
                            if (a.intersects(b))
                            {
                                handleCollision(i, j);

                                if (invisibleFramesLeft[i] > 0 || collisionCooldown[i] > 0)
                                {
                                    iConsumed = true; // i가 이번 충돌로 재생성/쿨다운에 들어갔으면 남은 이웃 셀은 볼 필요 없음
                                    break;
                                }

                                a = Sphere(positions[i], radii[i]); // i가 반응으로 움직였을 수 있으니 갱신
                            }
                        }
                    }
                }
            }
        }
    }

    void CalculatePhysicsSimulation::handleCollision(cUint32_t i, cUint32_t j)
    {
        // if (1)
        if (random::coinFlip())
        {

            // 패턴 C: 1차원 성분 분해 충격량 공식 (동일 질량 탄성 충돌)
            cVec3 distVec = positions[j] - positions[i];
            cFloat dist = glm::length(distVec);
            cVec3 n = glm::normalize(distVec);
            
            cVec3 vRel = velocities[j] - velocities[i];
            cFloat vNormalScalar = glm::dot(vRel, n);

            if (vNormalScalar > 0.0f)
                return; // 서로 멀어지는 중이면 반사 안 함
                
            cFloat jScalar = (-(1.0f + config.restitution) * vNormalScalar) / 2.0f; // 질량이 동일, 1로 가정
            cVec3 impulse = jScalar * n;

            velocities[i] -= impulse;
            velocities[j] += impulse;

            // 반사 직후에도 몇 프레임은 여전히 맞닿아 있을 수 있음 — 그동안은 재판정(재추첨) 안 하게 쿨다운
            collisionCooldown[i] = config.collisionCooldownFrames;
            collisionCooldown[j] = config.collisionCooldownFrames;
        }
        else
        {
            // 패턴 B: 파괴 후 재생성
            respawn(i);
            respawn(j);
        }
    }

    void CalculatePhysicsSimulation::respawn(cUint32_t i)
    {
        invisibleFramesLeft[i] = config.respawnInvisibleFrames;

        // 반지름만큼 안쪽으로 인셋해서, 재생성 직후 바로 벽에 겹치지 않게 함
        cVec3 extents = glm::max(config.worldBounds.getExtents() - cVec3(radii[i]), cVec3(0.0f));
        AABB spawnBounds = AABB::fromCenterExtents(config.worldBounds.getCenter(), extents);

        positions[i] = random::pointIn(spawnBounds);
        velocities[i] = random::unitDirection() * random::randomFloat(config.minSpeed, config.maxSpeed);
    }

    cIvec3 CalculatePhysicsSimulation::worldToCell(const cVec3 &pos) const
    {
        return glm::ivec3(
            static_cast<int>(std::floor(pos.x / cellSize)),
            static_cast<int>(std::floor(pos.y / cellSize)),
            static_cast<int>(std::floor(pos.z / cellSize)));
    }

    cInt64_t CalculatePhysicsSimulation::cellKey(const cIvec3 &cell) const
    {
        constexpr cInt64_t bias = 1 << 20; // 좌표 범위 ±100만 정도면 충분히 넉넉함
        cInt64_t x = cell.x + bias;
        cInt64_t y = cell.y + bias;
        cInt64_t z = cell.z + bias;
        return x | (y << 21) | (z << 42); // 축당 21비트
    }

    cUint32_t CalculatePhysicsSimulation::ObjectCount() const
    {
        return config.objectCount;
    }

    const cVec3 &CalculatePhysicsSimulation::Position(cUint32_t i) const
    {
        return positions[i];
    }

    cFloat CalculatePhysicsSimulation::Radius(cUint32_t i) const
    {
        return radii[i];
    }

    cBool CalculatePhysicsSimulation::IsVisible(cUint32_t i) const
    {
        return invisibleFramesLeft[i] == 0;
    }
}
