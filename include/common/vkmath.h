#ifndef INCLUDE_MATH_H_
#define INCLUDE_MATH_H_

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkMath
{
    // #DiredctXMath.h 에서 가져온 상수
    constexpr float XM_PI = 3.141592654f;
    constexpr float XM_2PI = 6.283185307f;
    constexpr float XM_1DIVPI = 0.318309886f;
    constexpr float XM_1DIV2PI = 0.159154943f;
    constexpr float XM_PIDIV2 = 1.570796327f;
    constexpr float XM_PIDIV4 = 0.785398163f;

    inline const glm::mat4 CreateRotationY(float radians)
    {
        float cosY = glm::cos(radians);
        float sinY = glm::sin(radians);
        glm::mat4 rotationY{
            cosY, 0.0f, -sinY, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sinY, 0.0f, cosY, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return rotationY;
    }
    inline const glm::mat4 CreateRotationX(float radians)
    {
        float cosX = glm::cos(radians);
        float sinX = glm::sin(radians);

        glm::mat4 rotationX{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosX, sinX, 0.0f,
            0.0f, -sinX, cosX, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        return rotationX;
    }
    inline const glm::mat4 CreateRotationZ(float radians)
    {
        float cosZ = glm::cos(radians);
        float sinZ = glm::sin(radians);
        glm::mat4 rotationZ{
            cosZ, -sinZ, 0.0f, 0.0f,
            sinZ, cosZ, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        return rotationZ;
    }

    inline const glm::mat4 fromQuatToMatrix(glm::quat quat)
    {
        return glm::mat4(
            1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z), 2.0f * (quat.x * quat.y - quat.w * quat.z), 2.0f * (quat.x * quat.z + quat.w * quat.y), 0.0f,
            2.0f * (quat.x * quat.y + quat.w * quat.z), 1.0f - 2.0f * (quat.x * quat.x + quat.z * quat.z), 2.0f * (quat.y * quat.z - quat.w * quat.x), 0.0f,
            2.0f * (quat.x * quat.z - quat.w * quat.y), 2.0f * (quat.y * quat.z + quat.w * quat.x), 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    const glm::mat4 CreateRotation(float yaw, float pitch, float roll);

    const glm::vec3 RotationQuat(const glm::quat q, const glm::vec3 dir);

    const glm::mat4 convertQuatToMatrix(const glm::quat q);

}

#endif // !INCLUDE_MATH_H_
