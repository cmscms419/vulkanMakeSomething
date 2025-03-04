#include "Camera.h"

#include "../math_.h"

using namespace vkMath;

namespace vkengine {
    namespace object {

        Camera::Camera() {
            pos = glm::vec3(0.0f, 0.0f, 3.0f);
            right = glm::vec3(1.0f, 0.0f, 0.0f);
            dir = glm::vec3(0.0f, 0.0f, -1.0f);
            up = glm::normalize(glm::cross(right, dir));

            fov = 45.0f;
            aspect = (float)WIDTH / (float)HEIGHT;
            nearP = 0.1f;
            farP = 1000.0f;
            
            speed = 2.5f;
            sensitivity = 0.1f;

            yaw = 0.0f;
            pitch = 0.0f;
            
            this->setViewDirection(pos, dir, up);
            this->setPerspectiveProjection(glm::radians(fov), aspect, nearP, farP);
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
            this->setViewDirection(pos, dir, up);
#if DEBUG_
            //printf("\rCamera Position: %f %f %f", this->pos.x, this->pos.y, this->pos.z);
            printf("\rCamera Direction: %f %f %f", this->dir.x, this->dir.y, this->dir.z);
#endif
        }
        
        void Camera::MoveForward(float deltaTime)
        {
            this->pos += this->dir * this->speed * deltaTime;
        }

        void Camera::MoveRight(float deltaTime)
        {
            this->pos += this->right * this->speed * deltaTime;
        }

        void Camera::MoveUp(float deltaTime)
        {
            this->pos += this->up * this->speed * deltaTime;
        }

        void Camera::MoveRotate(int xpos, int ypos, int windowWidth, int windowHeight)
        {
            // 마우스 커서의 위치를 NDC로 변환
            // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
            // NDC는 좌측 상단이 (-1, -1), 우측 하단(1, 1)
            float x = xpos * 2.0f / windowWidth - 1.0f;
            float y = ypos * 2.0f / windowHeight - 1.0f;

            // 커서가 화면 밖으로 나갔을 경우 범위 조절
            // 게임에서는 클램프를 안할 수도 있습니다.
            x = glm::clamp(x, -1.0f, 1.0f);
            y = glm::clamp(y, -1.0f, 1.0f);

            this->yaw = x * vkMath::XM_2PI;
            this->pitch = y * vkMath::XM_PIDIV2 * -1.f;

            // 방향 벡터 회전
            this->dir = glm::vec3(vkMath::CreateRotationY(this->yaw) * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            this->dir = glm::vec3(vkMath::CreateRotationX(this->pitch) * glm::vec4(this->dir, 1.0f));
            
            this->right = glm::cross(this->dir, glm::vec3(0.0f, 1.0f, 0.0f));

#if 0
            static bool firstMouse = true;
            static float lastX = 0.0f;
            static float lastY = 0.0f;
            
            if (firstMouse)
            {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }
            
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos;
            
            lastX = xpos;
            lastY = ypos;
            
            xoffset *= this->sensitivity;
            yoffset *= this->sensitivity;
            
            this->yaw += xoffset;
            this->pitch += yoffset;
            
            if (this->pitch > 89.0f)
                this->pitch = 89.0f;
            
            if (this->pitch < -89.0f)
                this->pitch = -89.0f;
            
            glm::vec3 front;
            
            front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
            front.y = sin(glm::radians(this->pitch));
            front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
            
            this->dir = glm::normalize(front);
            this->right = glm::normalize(glm::cross(this->dir, glm::vec3(0.0f, 1.0f, 0.0f)));
            this->up = glm::normalize(glm::cross(this->right, this->dir));
#endif
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
            
            /**
            * @brief Adjusts the y-axis direction for Vulkan coordinate system.
            *
            * In Vulkan, the y-axis is inverted compared to OpenGL.
            * This line multiplies the y-axis scale factor by -1 to correct the direction.
            * Vulkan에서는 OpenGL에 비해 y축이 반전됩니다.
            * 이 선은 y축 배율 인수에 -1을 곱하여 방향을 수정합니다.
            */
            projectionMatrix[1][1] *= -1.f;
        }

        void Camera::setViewDirection(glm::vec3 pos, glm::vec3 dir, glm::vec3 up)
        {
            const glm::vec3 w{ glm::normalize(dir) };
            const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
            const glm::vec3 v{ glm::cross(u, w) };

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