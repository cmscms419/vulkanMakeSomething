#include "Camera.h"

using namespace vkMath;

namespace vkengine {
    namespace object {

        Camera::Camera() {

            this->right = glm::cross(this->dir, this->up);

            this->Yaxis = this->up;
            this->Xaxis = -this->right;

            this->setViewDirection(pos, dir, up);
            this->setPerspectiveProjection(fov, aspect, nearP, farP);
            
            _PRINT_TO_CONSOLE_("Camera Position: %f %f %f\n", this->pos.x, this->pos.y, this->pos.z);
            _PRINT_TO_CONSOLE_("Camera Target: %f %f %f\n", this->target.x, this->target.y, this->target.z);
            _PRINT_TO_CONSOLE_("Camera Direction: %f %f %f\n", this->dir.x, this->dir.y, this->dir.z);
            _PRINT_TO_CONSOLE_("Camera Right: %f %f %f\n", this->right.x, this->right.y, this->right.z);
        }
        
        Camera::Camera(cVec3 pos, cVec3 up, cVec3 dir)
        {
            this->pos = pos;
            this->up = up;
            this->dir = dir;
            this->right = glm::cross(this->dir, this->up);

            this->Yaxis = this->up;
            this->Xaxis = -this->right;

            this->setViewDirection(this->pos, this->dir, this->up);
            this->setPerspectiveProjection(fov, aspect, nearP, farP);

            _PRINT_TO_CONSOLE_("Camera Position: %f %f %f\n", this->pos.x, this->pos.y, this->pos.z);
            _PRINT_TO_CONSOLE_("Camera Target: %f %f %f\n", this->target.x, this->target.y, this->target.z);
            _PRINT_TO_CONSOLE_("Camera Direction: %f %f %f\n", this->dir.x, this->dir.y, this->dir.z);
            _PRINT_TO_CONSOLE_("Camera Right: %f %f %f\n", this->right.x, this->right.y, this->right.z);
        }

        void Camera::update() {
            this->setViewDirection(pos, dir, up);
        }

        void Camera::RotateScreenStandard(cFloat xpos, cFloat ypos, int windowWidth, int windowHeight)
        {
            // 마우스 커서의 위치를 NDC로 변환
            // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
            // NDC는 좌측 상단이 (-1, -1), 우측 하단(1, 1)
            cFloat x = xpos * 2.0f / windowWidth - 1.0f;
            cFloat y = ypos * 2.0f / windowHeight - 1.0f;

            // 커서가 화면 밖으로 나갔을 경우 범위 조절
            // 게임에서는 클램프를 안할 수도 있습니다.
            //x = glm::clamp(x, -1.0f, 1.0f);
            //y = glm::clamp(y, -1.0f, 1.0f);

            this->yaw = x * vkMath::XM_PI; // 0 ~ 180도 마우스 회전 -> Y축 기준으로 회전
            this->pitch = y * vkMath::XM_PIDIV4; // 0 ~ 180도 마우스 회전 -> X축 기준으로 회전

            // 방향 벡터 회전
            cVec3 dirction(0.0f, 0.0f, -1.0f);
#if 1
            // 각 축별 quaternion 생성
            cQuat qYaw = glm::angleAxis(this->yaw, this->Yaxis); // Y축
            cQuat qPitch = glm::angleAxis(this->pitch, this->Xaxis); // X축

            cQuat Result = qYaw * qPitch; 
            dirction = vkMath::RotationQuat(Result, dirction);
            
            this->dir = glm::normalize(dirction);
            this->right = glm::normalize(glm::cross(this->dir, this->up));
#else
            dirction = cVec3(vkMath::CreateRotation(this->yaw,this->pitch, 0.0f) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
#endif

        }

        void Camera::RotateDeltaRotation(const cVec3& force, bool constrainPitch)
        {
            cFloat yawDelta = force.x * this->sensitivity;
            cFloat pitchDelta = force.y * this->sensitivity;

            this->yaw += yawDelta;
            this->pitch += pitchDelta;
            
            // 제한된 피치 각도
            if (constrainPitch)
            {
                this->pitch = glm::clamp(this->pitch, -MAX_PITCH_VALUE, MAX_PITCH_VALUE);
            }

            cQuat qYaw = glm::angleAxis(this->yaw, this->Yaxis); // Y축
            cQuat qPitch = glm::angleAxis(this->pitch, this->Xaxis); // X축

            cVec3 dirction(0.0f, 0.0f, -1.0f);

            cQuat Result = qYaw * qPitch;
            dirction = vkMath::RotationQuat(Result, dirction);

            // dirction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
            // dirction.y = sin(glm::radians(this->pitch));
            // dirction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

            this->dir = glm::normalize(dirction);
            this->right = glm::normalize(glm::cross(this->dir, this->up));
        }

        void Camera::setPerspectiveProjection(cFloat fov, cFloat aspect, cFloat nearP, cFloat farP)
        {
            this->fov = fov;
            this->aspect = aspect;
            this->nearP = nearP;
            this->farP = farP;

            this->setPerspectiveProjection();
        }

        void Camera::setOrthographicProjection(cFloat left, cFloat right, cFloat top, cFloat bottom, cFloat nearP, cFloat farP)
        {
            this->left = left;
            this->right_ = right;
            this->top = top;
            this->bottom = bottom;
            this->nearP = nearP;
            this->farP = farP;

            this->setOrthographicProjection();
        }

        void Camera::setPerspectiveProjection()
        {
            assert(glm::abs(aspect - std::numeric_limits<cFloat>::epsilon()) > 0.0f);
            
            const cFloat tanHalfFovy = tan(glm::radians(this->fov) / 2.f);

            projectionMatrix = cMat4{ 0.0f };
            projectionMatrix[0][0] = 1.f / (this->aspect * tanHalfFovy);
            projectionMatrix[1][1] = 1.f / (tanHalfFovy);
            projectionMatrix[2][2] = this->farP / (this->nearP - this->farP);
            projectionMatrix[2][3] = -1.f;
            projectionMatrix[3][2] = -(this->farP * this->nearP) / (this->farP - this->nearP);

            /**
            * @brief Adjusts the y-axis direction for Vulkan coordinate system.
            *
            * In Vulkan, the y-axis is inverted compared to OpenGL.
            * This line multiplies the y-axis scale factor by -1 to correct the direction.
            *
            * Vulkan에서는 OpenGL에 비해 y축이 반전됩니다.
            * 이 선은 y축 배율 인수에 -1을 곱하여 방향을 수정합니다.
            */

            projectionMatrix[1][1] *= -1.f;
        }

        void Camera::setOrthographicProjection()
        {
            projectionMatrix = glm::mat4{ 1.0f };

            projectionMatrix[0][0] = 2.f / (this->right_ - this->left);
            projectionMatrix[1][1] = 2.f / (this->bottom - this->top);
            projectionMatrix[2][2] = 1.f / (this->nearP - this->farP);
            projectionMatrix[3][0] = -(this->right_ + this->left) / (this->right_ - this->left);
            projectionMatrix[3][1] = -(this->bottom + this->top) / (this->bottom - this->top);
            projectionMatrix[3][2] = -this->nearP / (this->farP - this->nearP);
        }

        void Camera::setViewDirection(cVec3 pos, cVec3 dir, cVec3 up)
        {
            const cVec3 w{ glm::normalize(dir) };
            const cVec3 u{ glm::normalize(glm::cross(w, up)) };
            const cVec3 v{ glm::cross(w, u) };

            viewMatrix = cMat4{ 1.f };
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

        void Camera::setViewTarget(cVec3 pos, cVec3 target, cVec3 up)
        {
            setViewDirection(pos, target - pos, up);
        }

        void Camera::setViewXYZ(cVec3 pos, cVec3 rot)
        {
            const cFloat c3 = glm::cos(rot.z);
            const cFloat s3 = glm::sin(rot.z);
            const cFloat c2 = glm::cos(rot.x);
            const cFloat s2 = glm::sin(rot.x);
            const cFloat c1 = glm::cos(rot.y);
            const cFloat s1 = glm::sin(rot.y);
            const cVec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
            const cVec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
            const cVec3 w{ (c2 * s1), (-s2), (c1 * c2) };
            viewMatrix = cMat4{ 1.f };
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