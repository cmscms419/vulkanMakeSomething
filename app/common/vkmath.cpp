#include "VKmath.h"

namespace vkMath {
const cMat4 CreateRotation(cFloat yaw, cFloat pitch, cFloat roll) {
  return CreateRotationY(yaw) * CreateRotationX(pitch) * CreateRotationZ(roll);
}

const cVec3 RotationQuat(const cQuat q, const cVec3 dir) {
  cQuat quatDir(0.0f, dir.x, dir.y, dir.z);
  cQuat qConjugate(q.w, -q.x, -q.y, -q.z);
  cQuat quatRotdir = q * quatDir * qConjugate;  // q*v*q_
  return cVec3(quatRotdir.x, quatRotdir.y, quatRotdir.z);
}
const cMat4 convertQuatToMatrix(const cQuat q) {
  return glm::mat4_cast(q);
}
}  // namespace vkMath