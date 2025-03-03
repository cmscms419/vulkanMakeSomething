#ifndef INCLUDE_CAMERA_H_
#define INCLUDE_CAMERA_H_

#include "../_common.h"
#include "../struct.h"

namespace vkengine {
    namespace object {

        class Camera {

        public:
            Camera();
            ~Camera();
            void update();

            void setPos(glm::vec3 pos);
            glm::vec3 getTarget() { return this->target; }
            glm::vec3 getPos() { return this->pos; }
            void MoveForward(float deltaTime);
            void MoveRight(float deltaTime);

            void setPerspectiveProjection(float fov, float aspect, float nearP, float farP);

            const glm::mat4& getProjectionMatrix() { return this->projectionMatrix; }
            const glm::mat4& getViewMatrix() { return this->viewMatrix; }

            void setViewDirection(glm::vec3 pos, glm::vec3 dir, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
            void setViewTarget(glm::vec3 pos, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
            void setViewXYZ(glm::vec3 pos, glm::vec3 rot);

        private:
            glm::vec3 pos;
            glm::vec3 up;
            glm::vec3 target;
            glm::vec3 dir;
            glm::vec3 right;

            float yaw;
            float pitch;
            float roll;
            float fov;
            float aspect;
            float nearP;
            float farP;
            float speed;
            float sensitivity;

            glm::mat4 viewMatrix{ 1.f };
            glm::mat4 projectionMatrix{ 1.f };
       };
    }
}

#endif // INCLUDE_CAMERA_H_