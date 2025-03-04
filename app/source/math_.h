#ifndef INCLUDE_MATH_H_
#define INCLUDE_MATH_H_

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace vkMath
{
    // #DiredctXMath.h 에서 가져온 상수
    constexpr float XM_PI = 3.141592654f;
    constexpr float XM_2PI = 6.283185307f;
    constexpr float XM_1DIVPI = 0.318309886f;
    constexpr float XM_1DIV2PI = 0.159154943f;
    constexpr float XM_PIDIV2 = 1.570796327f;
    constexpr float XM_PIDIV4 = 0.785398163f;

    const glm::mat4 CreateRotationY(float radians) {
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

    const glm::mat4 CreateRotationX(float radians) {
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
}

#endif // !INCLUDE_MATH_H_
