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
            explicit Camera(glm::vec3 pos, glm::vec3 up, glm::vec3 dir);
            ~Camera();

            void update();

            void setPos(glm::vec3 pos);
            const glm::vec3 getTarget() { return this->target; }
            const glm::vec3 getPos() { return this->pos; }
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
            void RotateDeltaRotation(const glm::vec3& force, bool constrainPitch = true);

            void setPerspectiveProjection(float fov, float aspect, float nearP, float farP);

            const glm::mat4& getProjectionMatrix() { return this->projectionMatrix; }
            const glm::mat4& getViewMatrix() { return this->viewMatrix; }

            void setViewDirection(glm::vec3 pos, glm::vec3 dir, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
            void setViewTarget(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
            void setViewXYZ(glm::vec3 pos, glm::vec3 rot);

        private:
            glm::vec3 pos{ glm::vec3(0.0f, 0.0f, 3.0f) };
            glm::vec3 up{ glm::vec3(0.0f, 1.0f, 0.0f) };
            glm::vec3 target{ glm::vec3(0.0f) };
            glm::vec3 dir{ glm::vec3(0.0f, 0.0f, -1.0f) };
            glm::vec3 right{ glm::vec3(1.0f, 0.0f, 0.0f) };

            float yaw{ 0.0f };
            float pitch{ 0.0f };
            float fov{ 45.f };
            float aspect = []() { return (float)WIDTH / (float)HEIGHT; }();
            float nearP{ 0.1f };
            float farP{ 1000.0f };
            float speed{ 2.5f };;
            float sensitivity{ 0.001f };

            glm::mat4 viewMatrix{ 1.f };
            glm::mat4 projectionMatrix{ 1.f };
        };
    }
}

#endif // INCLUDE_CAMERA_H_