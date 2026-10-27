#ifndef INCLUDE_RANDOM_H_
#define INCLUDE_RANDOM_H_

#include "common.h"
#include "geometry.h"

#include <random>

namespace vkengine
{
    namespace random
    {
        // Returns a random float in the range [min, max]
        inline cFloat generateRandomFloat(cFloat min, cFloat max)
        {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<cFloat> dis(min, max);
            return dis(gen);
        }

        // 50/50 판정 (충돌 반응 패턴 A/B 선택 등에 사용)
        inline cBool flipCoin()
        {
            return generateRandomFloat(0.0f, 1.0f) < 0.5f;
        }

        // AABB 내부의 무작위 지점 (재생성 위치 등에 사용)
        inline cVec3 generatePointIn(const AABB &bounds)
        {
            return cVec3(
                generateRandomFloat(bounds.min.x, bounds.max.x),
                generateRandomFloat(bounds.min.y, bounds.max.y),
                generateRandomFloat(bounds.min.z, bounds.max.z));
        }

        // 구면 위 무작위 단위 벡터 (재생성 속도 방향 등에 사용)
        inline cVec3 generateUnitDirection()
        {
            cVec3 v = glm::normalize(cVec3(generateRandomFloat(-1.0f, 1.0f), generateRandomFloat(-1.0f, 1.0f), generateRandomFloat(-1.0f, 1.0f)));
            return v;
        }
    }
}

#endif // !INCLUDE_RANDOM_H_
