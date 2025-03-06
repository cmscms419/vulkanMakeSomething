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

    const glm::mat4 CreateRotationY(float radians);
    const glm::mat4 CreateRotationX(float radians);
    const glm::mat4 CreateRotationZ(float radians);
    
    const glm::mat4 CreateRotation(float yaw, float pitch, float roll);
    
    const glm::vec3 RotationQuat(const glm::quat q, const glm::vec3 dir);

}

#endif // !INCLUDE_MATH_H_
