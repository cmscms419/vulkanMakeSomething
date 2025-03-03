#ifndef INCLUDE_CAMERA_H_
#define INCLUDE_CAMERA_H_

#include "../_common.h"
#include "../struct.h"

namespace vkutil {
    namespace object {

        struct UniformBufferObject {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        class Camera {


        public:
            Camera();
            ~Camera();
            void update();
            void setProjection(float fov, float aspect, float near, float far);
            void setView(glm::vec3 pos, glm::vec3 target, glm::vec3 up);
            void setSpeed(float speed);
            void setSensitivity(float sensitivity);
            void setFov(float fov);
            void setAspect(float aspect);
            void setNear(float near);
            void setFar(float far);
            void setPos(glm::vec3 pos);
            void setTarget(glm::vec3 target);
            void setUp(glm::vec3 up);
            void setFront(glm::vec3 front);
            void setRight(glm::vec3 right);
            void setYaw(float yaw);
            void setPitch(float pitch);
            void setRoll(float roll);
            void setViewMatrix(glm::mat4 view);
            void setProjectionMatrix(glm::mat4 projection);
            void setViewProjectionMatrix(glm::mat4 viewProjection);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse, glm::mat4 projectionInverseTranspose);
            void setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse, glm::mat4 projectionInverseTranspose, glm::mat4 viewInverseTranspose);

            glm::mat4 getViewMatrix();
            glm::vec3 getView();

            void MoveForward(float deltaTime);
            void MoveRight(float deltaTime);

            glm::mat4 getProjectionMatrix();
            void getProjection(float& fov, float& aspect, float& nearP, float& farP);


        private:
            glm::vec3 pos;
            glm::vec3 up;
            glm::vec3 front;
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

            glm::mat4 view;
            glm::mat4 projection;
            glm::mat4 viewProjection;
            glm::mat4 viewProjectionInverse;
            glm::mat4 viewInverse;
            glm::mat4 projectionInverse;
            glm::mat4 projectionInverseTranspose;
            glm::mat4 viewInverseTranspose;
        };
    }
}

#endif // INCLUDE_CAMERA_H_