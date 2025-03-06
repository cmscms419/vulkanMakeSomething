#include "vkmath.h"

namespace vkMath
{
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
	const glm::mat4 CreateRotationZ(float radians) {
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

	const glm::mat4 CreateRotation(float yaw, float pitch, float roll) {
		return CreateRotationY(yaw) * CreateRotationX(pitch) * CreateRotationZ(roll);
	}

	const glm::vec3 RotationQuat(const glm::quat q, const glm::vec3 dir)
	{
		glm::quat quatDir(0.0f, dir.x, dir.y, dir.z);
		glm::quat qConjugate(q.w, -q.x, -q.y, -q.z);
		glm::quat quatRotdir = q * quatDir * qConjugate; //q*v*q_
		return glm::vec3(quatRotdir.x, quatRotdir.y, quatRotdir.z);
	}
}