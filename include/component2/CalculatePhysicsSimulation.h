#ifndef INCLUDE_COMPONENT2_CALCULATEPHYSICSSIMULATION_H_
#define INCLUDE_COMPONENT2_CALCULATEPHYSICSSIMULATION_H_

#include "common.h"
#include "geometry.h"

#include <vector>
#include <unordered_map>

namespace vkengine
{
    class CalculatePhysicsSimulation
    {
    public:
        struct Config
        {
            cUint32_t objectCount;
            cFloat objectRadius;
            AABB worldBounds;
            cFloat restitution = 1.0f;
            cFloat minSpeed = 1.0f, maxSpeed = 3.0f;
            cUint32_t respawnInvisibleFrames = 1;  // 패턴B: 몇 프레임 동안 안 보이는지
            cUint32_t collisionCooldownFrames = 6; // 패턴A(반사) 직후, 같은 접촉이 재판정되지 않도록 잠깐 쉬는 프레임 수
        };

        CalculatePhysicsSimulation(const Config &cfg);
        ~CalculatePhysicsSimulation();

        void update(cFloat deltaTime);

        cUint32_t ObjectCount() const;
        const cVec3 &Position(cUint32_t i) const;
        cFloat Radius(cUint32_t i) const;
        cBool IsVisible(cUint32_t i) const; // respawn 대기 중이면 false

    private:
        std::vector<cVec3> positions, velocities;
        std::vector<cFloat> radii; // 지금은 전부 동일값이지만 향후 크기 다양화 대비 배열로
        std::vector<cUint32_t> invisibleFramesLeft;
        std::vector<cUint32_t> collisionCooldown; // >0이면 이번 프레임 충돌 판정에서 제외 (연속 재판정 방지)

        Config config;
        cFloat cellSize;
        std::unordered_map<cInt64_t, std::vector<cUint32_t>> grid; // 4-b에서 사용, 4-a는 비워두고 전수 비교

        void integrate(cFloat dt); // 통합 -> 위치, 속도 등 계산한 결과를 정리해서 적용시킨다
        void resolveWorldBoundary();
        void rebuildGrid();       // 4-b
        void resolveCollisions(); // 4-a: 전수 O(n²) / 4-b: 그리드 순회
        void handleCollision(cUint32_t i, cUint32_t j);
        void respawn(cUint32_t i);
        
        cIvec3 worldToCell(const cVec3 &pos) const;
        cInt64_t cellKey(const cIvec3 &cell) const;
    };
}

#endif // !INCLUDE_COMPONENT2_CALCULATEPHYSICSSIMULATION_H_