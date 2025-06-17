#ifndef INCLUDE_CAMERA_H_
#define INCLUDE_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "vkmath.h"
#include "macros.h"

#define MAX_PITCH_VALUE 1.3f

namespace vkengine {
    namespace object {

        class Camera {

        public:
            explicit Camera();
            explicit Camera(cVec3 pos, cVec3 up, cVec3 dir);
            ~Camera();

            void update();

            const cVec3 getTarget() { return this->target; }
            const cVec3 getDir() { return this->dir; }
            const cVec3 getPos() { return this->pos; }
            const float getYaw() { return this->yaw; }
            const float getPitch() { return this->pitch; }
            const float getFov() { return this->fov; }
            const float getAspect() { return this->aspect; }
            const float getNearP() { return this->nearP; }
            const float getFarP() { return this->farP; }

            void setPos(cVec3 pos) { this->pos = pos; }
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
            cVec3 pos{ cVec3(0.0f, 0.0f, -3.0f) };
            cVec3 up{ cVec3(0.0f, 1.0f, 0.0f) };
            cVec3 target{ cVec3(0.0f) };
            cVec3 dir{ cVec3(0.0f, 0.0f, 1.0f) };
            cVec3 right{ cVec3(1.0f, 0.0f, 0.0f) };

            cFloat yaw{ 0.0f };
            cFloat pitch{ 0.0f };
            cFloat fov{ 45.f };
            cFloat aspect = []() { return (float)WIDTH / (float)HEIGHT; }();
            cFloat nearP{ 0.1f };
            cFloat farP{ 1000.0f };
            cFloat speed{ 2.5f };
            cFloat sensitivity{ 0.001f };
            cBool flipY{ false };

            cMat4 viewMatrix{ 1.f };
            cMat4 projectionMatrix{ 1.f };
        };
    }
}

#endif // INCLUDE_CAMERA_H_