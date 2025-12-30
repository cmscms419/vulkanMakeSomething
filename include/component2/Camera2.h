#ifndef  INCLUDE_CAMERA_2_H_
#define  INCLUDE_CAMERA_2_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common.h"
#include "vkmath.h"

#include "log.h"

#define MAX_PITCH_VALUE 1.5f

namespace vkengine {
    namespace object {
        class Camera2 {

        public:
            explicit Camera2();
            explicit Camera2(cVec3 pos, cVec3 up, cVec3 dir, cVec3 right);
            ~Camera2() {}

            void update();

            const cVec3 getTarget() { return this->target; }
            const cVec3 getDir() { return this->dir; }
            const cVec3 getPos() { return this->pos; }
            const cFloat getYaw() { return this->yaw; }
            const cFloat getPitch() { return this->pitch; }
            const cFloat getFov() { return this->fov; }
            const cFloat getAspect() { return this->aspect; }
            const cFloat getNearP() { return this->nearP; }
            const cFloat getFarP() { return this->farP; }

            void setPos(cVec3 pos) { this->pos = pos; }
            void setYaw(cFloat yaw) { this->yaw = yaw; }
            void setPitch(cFloat pitch) { this->pitch = pitch; }
            void setFov(cFloat fov) { this->fov = fov; }

            void MoveForward(cFloat deltaTime)
            {
                this->pos -= this->dir * this->speed * deltaTime;
            };

            void MoveRight(cFloat deltaTime)
            {
                this->pos += this->right * this->speed * deltaTime;
            };

            void MoveUp(cFloat deltaTime)
            {
                this->pos -= this->up * this->speed * deltaTime;
            };

            void RotateScreenStandard(cFloat xpos, cFloat ypos, int windowWidth, int windowHeight);
            void RotateDeltaRotation(const cVec3& force, bool constrainPitch = true);

            void setPerspectiveProjection(cFloat fov, cFloat aspect, cFloat nearP, cFloat farP);
            void setPerspectiveProjection();

            void setOrthographicProjection(cFloat left, cFloat right, cFloat top, cFloat bottom, cFloat nearP, cFloat farP);
            void setOrthographicProjection();

            const cMat4& getProjectionMatrix() { return this->projectionMatrix; }
            const cMat4& getViewMatrix() { return this->viewMatrix; }

            void setViewDirection(cVec3 pos, cVec3 dir, cVec3 up = cVec3(0.f, 1.f, 0.f));
            void setViewTarget(cVec3 pos, cVec3 target, cVec3 up = cVec3(0.f, 1.f, 0.f));
            void setViewXYZ(cVec3 pos, cVec3 rot);

        private:
            cVec3 pos{ cVec3(0.0f, 0.0f, -3.0f) };
            cVec3 up{ cVec3(0.0f, -1.0f, 0.0f) };
            cVec3 target{ cVec3(0.0f) };
            cVec3 dir{ cVec3(0.0f, 0.0f, -1.0f) };
            cVec3 right{ cVec3(-1.0f, 0.0f, 0.0f) };

            cVec3 Yaxis = cVec3(0.0f, 1.0f, 0.0f); // Y√‡ πÊ«‚ ∫§≈Õ
            cVec3 Xaxis = cVec3(1.0f, 0.0f, 0.0f); // X√‡ πÊ«‚ ∫§≈Õ
            cVec3 Zaxis = cVec3(0.0f, 0.0f, 1.0f); // Z√‡ πÊ«‚ ∫§≈Õ

            cFloat yaw{ 0.0f };
            cFloat pitch{ 0.0f };
            cFloat fov{ 45.f };
            cFloat aspect{ 0.f };
            cFloat nearP{ 0.1f };
            cFloat farP{ 1000.0f };
            cFloat left{ -1.0f };
            cFloat right_{ 1.0f };
            cFloat top{ 1.0f };
            cFloat bottom{ -1.0f };
            cFloat speed{ 1.5f };
            cFloat sensitivity{ 0.001f };

            cMat4 viewMatrix{ 1.f };
            cMat4 projectionMatrix{ 1.f };
        };
    }
}

#endif // ! INCLUDE_CAMERA_2_H_