#ifndef INCLUDE_MATH_H_
#define INCLUDE_MATH_H_

#include "base_types.h"
#include "math_types.h"
#include <cstdint>
#include <string>

namespace vkHash
{
    // FNV-1a 64bit — 컴파일 타임 사용: constexpr auto key = vkHash::fnv1a("MyTexture");
    constexpr cUint64_t fnv1a(const char* str, cUint64_t hash = 14695981039346656037ULL)
    {
        return *str ? fnv1a(str + 1, (hash ^ static_cast<cUint64_t>(*str)) * 1099511628211ULL) : hash;
    }

    // 런타임 사용: vkHash::fnv1a(name)
    inline cUint64_t fnv1a(const std::string& str)
    {
        cUint64_t hash = 14695981039346656037ULL;
        for (char c : str)
            hash = (hash ^ static_cast<cUint64_t>(c)) * 1099511628211ULL;
        return hash;
    }
}

namespace vkMath
{
    // #DiredctXMath.h 에서 가져온 상수
    constexpr cFloat XM_PI = 3.141592654f;
    constexpr cFloat XM_2PI = 6.283185307f;
    constexpr cFloat XM_1DIVPI = 0.318309886f;
    constexpr cFloat XM_1DIV2PI = 0.159154943f;
    constexpr cFloat XM_PIDIV2 = 1.570796327f;
    constexpr cFloat XM_PIDIV4 = 0.785398163f;

    inline const cMat4 CreateRotationY(cFloat radians)
    {
        cFloat cosY = glm::cos(radians);
        cFloat sinY = glm::sin(radians);
        cMat4 rotationY{
            cosY, 0.0f, -sinY, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinY, 0.0f, cosY, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return rotationY;
    }
    inline const cMat4 CreateRotationX(cFloat radians)
    {
        cFloat cosX = glm::cos(radians);
        cFloat sinX = glm::sin(radians);

        cMat4 rotationX{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosX, sinX, 0.0f,
            0.0f, -sinX, cosX, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return rotationX;
    }
    inline const cMat4 CreateRotationZ(cFloat radians)
    {
        cFloat cosZ = glm::cos(radians);
        cFloat sinZ = glm::sin(radians);
        cMat4 rotationZ{
            cosZ, -sinZ, 0.0f, 0.0f,
            sinZ, cosZ, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        return rotationZ;
    }

    inline const cMat4 fromQuatToMatrix(cQuat quat)
    {
        return cMat4(
            1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z), 2.0f * (quat.x * quat.y - quat.w * quat.z), 2.0f * (quat.x * quat.z + quat.w * quat.y), 0.0f,
            2.0f * (quat.x * quat.y + quat.w * quat.z), 1.0f - 2.0f * (quat.x * quat.x + quat.z * quat.z), 2.0f * (quat.y * quat.z - quat.w * quat.x), 0.0f,
            2.0f * (quat.x * quat.z - quat.w * quat.y), 2.0f * (quat.y * quat.z + quat.w * quat.x), 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    const cMat4 CreateRotation(cFloat yaw, cFloat pitch, cFloat roll);

    const cVec3 RotationQuat(const cQuat q, const cVec3 dir);

    const cMat4 convertQuatToMatrix(const cQuat q);

}

#endif // !INCLUDE_MATH_H_
