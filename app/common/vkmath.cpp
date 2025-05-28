#include "vkmath.h"

namespace vkMath {
const glm::mat4 CreateRotation(float yaw, float pitch, float roll) {
  return CreateRotationY(yaw) * CreateRotationX(pitch) * CreateRotationZ(roll);
}

const glm::vec3 RotationQuat(const glm::quat q, const glm::vec3 dir) {
  glm::quat quatDir(0.0f, dir.x, dir.y, dir.z);
  glm::quat qConjugate(q.w, -q.x, -q.y, -q.z);
  glm::quat quatRotdir = q * quatDir * qConjugate;  // q*v*q_
  return glm::vec3(quatRotdir.x, quatRotdir.y, quatRotdir.z);
}
const glm::mat4 convertQuatToMatrix(const glm::quat q) {
  return glm::mat4_cast(q);
}
}  // namespace vkMath