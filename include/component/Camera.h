#ifndef INCLUDE_CAMERA_H_
#define INCLUDE_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "vkmath.h"
#include "macros.h"

#define MAX_PITCH_VALUE 1.1f

namespace vkengine {
    namespace object {

        class Camera {

        public:
            explicit Camera();
            explicit Camera(cVec3 pos, cVec3 up, cVec3 dir);
            ~Camera();

            void update();

            void setPos(cVec3 pos);
            const cVec3 getTarget() { return this->target; }
            const cVec3 getPos() { return this->pos; }
            const float getYaw() { return this->yaw; }
            const float getPitch() { return this->pitch; }
            const float getFov() { return this->fov; }
            const float getAspect() { return this->aspect; }
            const float getNearP() { return this->nearP; }
            const float getFarP() { return this->farP; }

            void setYaw(float yaw) { this->yaw = yaw; }
            void setPitch(float pitch) { this->pitch = pitch; }
            void setFov(float fov) { this->fov = fov; }
            void MoveForward(float deltaTime);
            void MoveRight(float deltaTime);
            void MoveUp(float deltaTime);

            void RotateScreenStandard(float xpos, float ypos, int windowWidth, int windowHeight);
            void RotateDeltaRotation(const cVec3& force, bool constrainPitch = true);

            void setPerspectiveProjection(float fov, float aspect, float nearP, float farP);

            const cMat4& getProjectionMatrix() { return this->projectionMatrix; }
            const cMat4& getViewMatrix() { return this->viewMatrix; }

            void setViewDirection(cVec3 pos, cVec3 dir, cVec3 up = cVec3(0.f, 1.f, 0.f));
            void setViewTarget(cVec3 pos, cVec3 target, cVec3 up = cVec3(0.f, 1.f, 0.f));
            void setViewXYZ(cVec3 pos, cVec3 rot);

        private:
            cVec3 pos{ cVec3(0.0f, 0.0f, 3.0f) };
            cVec3 up{ cVec3(0.0f, 1.0f, 0.0f) };
            cVec3 target{ cVec3(0.0f) };
            cVec3 dir{ cVec3(0.0f, 0.0f, -1.0f) };
            cVec3 right{ cVec3(1.0f, 0.0f, 0.0f) };

            float yaw{ 0.0f };
            float pitch{ 0.0f };
            float fov{ 45.f };
            float aspect = []() { return (float)WIDTH / (float)HEIGHT; }();
            float nearP{ 0.1f };
            float farP{ 1000.0f };
            float speed{ 2.5f };;
            float sensitivity{ 0.001f };

            cMat4 viewMatrix{ 1.f };
            cMat4 projectionMatrix{ 1.f };
        };
    }
}

#endif // INCLUDE_CAMERA_H_