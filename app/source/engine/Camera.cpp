#include "Camera.h"

namespace vkengine {
    namespace object {

        Camera::Camera() {
            pos = glm::vec3(2.0f);
            right = glm::vec3(1.0f, 0.0f, 0.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            //target = glm::vec3(0.5f, 0.5f, -1.0f);
            //dir = glm::normalize(target - pos);
            dir = glm::cross(right, up) * -1.f;

            fov = 45.0f;
            aspect = (float)WIDTH / (float)HEIGHT;
            nearP = 0.1f;
            farP = 100.0f;
            
            speed = 2.5f;
            sensitivity = 0.1f;
            
            this->setViewDirection(pos, dir);
            this->setPerspectiveProjection(glm::radians(fov), aspect, nearP, farP);
            this->projectionMatrix[1][1] *= -1;
#if DEBUG_
            printf("Camera Position: %f %f %f\n", this->pos.x, this->pos.y, this->pos.z);
            printf("Camera Target: %f %f %f\n", this->target.x, this->target.y, this->target.z);
            printf("Camera Direction: %f %f %f\n", this->dir.x, this->dir.y, this->dir.z);
            printf("Camera Right: %f %f %f\n", this->right.x, this->right.y, this->right.z);
#endif
        }
        
        Camera::~Camera() {}

        void Camera::update() {
            // Implementation here
            
            //printf("Camera Position: %f %f %f\n", this->pos.x, this->pos.y, this->pos.z);
        }
        
        void Camera::MoveForward(float deltaTime)
        {
            this->pos += this->target * this->speed * deltaTime;
        }

        void Camera::MoveRight(float deltaTime)
        {
            this->pos += this->right * this->speed * deltaTime;
        }

        void Camera::setPerspectiveProjection(float fov, float aspect, float nearP, float farP)
        {
            assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

            const float tanHalfFovy = tan(fov / 2.f);
            projectionMatrix = glm::mat4{ 0.0f };
            projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
            projectionMatrix[1][1] = 1.f / (tanHalfFovy);
            projectionMatrix[2][2] = farP / (farP - nearP);
            projectionMatrix[2][3] = 1.f;
            projectionMatrix[3][2] = -(farP * nearP) / (farP - nearP);
        }

        void Camera::setViewDirection(glm::vec3 pos, glm::vec3 dir, glm::vec3 up)
        {
            const glm::vec3 w{ glm::normalize(dir) };
            const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
            const glm::vec3 v{ glm::cross(w, u) };

            viewMatrix = glm::mat4{ 1.f };
            viewMatrix[0][0] = u.x;
            viewMatrix[1][0] = u.y;
            viewMatrix[2][0] = u.z;
            viewMatrix[0][1] = v.x;
            viewMatrix[1][1] = v.y;
            viewMatrix[2][1] = v.z;
            viewMatrix[0][2] = w.x;
            viewMatrix[1][2] = w.y;
            viewMatrix[2][2] = w.z;
            viewMatrix[3][0] = -glm::dot(u, pos);
            viewMatrix[3][1] = -glm::dot(v, pos);
            viewMatrix[3][2] = -glm::dot(w, pos);
        }

        void Camera::setViewTarget(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
        {
            setViewDirection(pos, target - pos, up);
        }

        void Camera::setViewXYZ(glm::vec3 pos, glm::vec3 rot)
        {
            const float c3 = glm::cos(rot.z);
            const float s3 = glm::sin(rot.z);
            const float c2 = glm::cos(rot.x);
            const float s2 = glm::sin(rot.x);
            const float c1 = glm::cos(rot.y);
            const float s1 = glm::sin(rot.y);
            const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
            const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
            const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
            viewMatrix = glm::mat4{ 1.f };
            viewMatrix[0][0] = u.x;
            viewMatrix[1][0] = u.y;
            viewMatrix[2][0] = u.z;
            viewMatrix[0][1] = v.x;
            viewMatrix[1][1] = v.y;
            viewMatrix[2][1] = v.z;
            viewMatrix[0][2] = w.x;
            viewMatrix[1][2] = w.y;
            viewMatrix[2][2] = w.z;
            viewMatrix[3][0] = -glm::dot(u, pos);
            viewMatrix[3][1] = -glm::dot(v, pos);
            viewMatrix[3][2] = -glm::dot(w, pos);
        }


    }
}