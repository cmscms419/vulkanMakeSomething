#ifndef INCLUDE_COMPONENT2_CALCULATEPHYSICSSIMULATION_H_
#define INCLUDE_COMPONENT2_CALCULATEPHYSICSSIMULATION_H_

#include "common.h"
#include "type.h"
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
            cUint32_t objectCount;    // 객체 개수 -> 실지적으로 랜더링 되어야 하는 개수
            cFloat objectRadius;      // Sphere 반지름 (모든 Sphere가 동일한 반지름을 가짐)
            AABB worldBounds;         // Box의 worldBounds (Sphere들이 이 안에서 움직임)
            cFloat restitution;       // 충돌 후 반사 계수 (1.0이면 완전 탄성 충돌)
            cFloat minSpeed;            // Sphere 초기 최소 속도
            cFloat maxSpeed;            // Sphere 초기 최대 속도
            cUint32_t respawnInvisibleFrames;  // 몇 프레임 동안 안 보이는지
            cUint32_t collisionCooldownFrames; // 반사 직후, 같은 접촉이 재판정되지 않도록 잠깐 쉬는 프레임 수
        };

        CalculatePhysicsSimulation(const Config &cfg);
        ~CalculatePhysicsSimulation();

        void update(cFloat deltaTime);
        void updateConfig(const Config &cfg);
        void updateInstanceData(std::vector<InstanceData>& dst) {
            std::swap(this->shaderData, dst);
        }

        cUint32_t ObjectCount() const;
        const cVec3 &Position(cUint32_t i) const;
        cFloat Radius() const;
        cBool IsVisible(cUint32_t i) const; // respawn 대기 중이면 false
        Config getConfig() const { return config; }
        

    private:
        std::vector<InstanceData> shaderData;
        std::vector<cVec3> positions, velocities;
        std::vector<cUint32_t> invisibleFramesLeft;
        std::vector<cUint32_t> collisionCooldown; // >0이면 이번 프레임 충돌 판정에서 제외 (연속 재판정 방지)

        Config config;
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