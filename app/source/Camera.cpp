#include "Camera.h"

namespace vkutil {
    namespace object {

        Camera::Camera() {
            pos = glm::vec3(0.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            front = glm::vec3(0.0f, 0.0f, -1.0f);
            right = glm::vec3(0.0f);
            
            yaw = -90.0f;
            pitch = 0.0f;
            roll = 0.0f;
            
            fov = 45.0f;
            aspect = (float)WIDTH / (float)HEIGHT;
            nearP = 0.1f;
            farP = 100.0f;
            
            speed = 2.5f;
            sensitivity = 0.1f;
            
            view = glm::lookAt(front, pos, up);
            projection = glm::perspective(glm::radians(fov), aspect, nearP, farP);
            viewProjection = projection * view;
            viewProjectionInverse = glm::inverse(viewProjection);
            viewInverse = glm::inverse(view);
            projectionInverse = glm::inverse(projection);
            viewInverseTranspose = glm::transpose(viewInverse);
            projectionInverseTranspose = glm::transpose(projectionInverse);
        }
        
        Camera::~Camera() {}

        void Camera::update() {
            // Implementation here
        }

        void Camera::setProjection(float fov, float aspect, float nearP, float farP) {
            // Implementation here
            this->aspect = aspect;
            this->fov = glm::radians(fov);
            this->nearP = nearP;
            this->farP = farP;

            this->projection = glm::perspective(this->fov, this->aspect, this->nearP, this->farP);
            this->projection[1][1] *= -1;
        }

        void Camera::setView(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {
            // Implementation here
            this->pos = pos;
            this->front = target;
            this->up = up;
            this->view = glm::lookAt(this->pos, this->front, this->up);
        }

        void Camera::setSpeed(float speed) {
            // Implementation here
        }

        void Camera::setSensitivity(float sensitivity) {
            // Implementation here
        }

        void Camera::setFov(float fov) {
            // Implementation here
        }

        void Camera::setAspect(float aspect) {
            // Implementation here
        }

        void Camera::setNear(float near) {
            // Implementation here
        }

        void Camera::setFar(float far) {
            // Implementation here
        }

        void Camera::setPos(glm::vec3 pos) {
            // Implementation here
        }

        void Camera::setTarget(glm::vec3 target) {
            // Implementation here
        }

        void Camera::setUp(glm::vec3 up) {
            // Implementation here
        }

        void Camera::setFront(glm::vec3 front) {
            // Implementation here
        }

        void Camera::setRight(glm::vec3 right) {
            // Implementation here
        }

        void Camera::setYaw(float yaw) {
            // Implementation here
        }

        void Camera::setPitch(float pitch) {
            // Implementation here
        }

        void Camera::setRoll(float roll) {
            // Implementation here
        }

        void Camera::setViewMatrix(glm::mat4 view) {
            // Implementation here
        }

        void Camera::setProjectionMatrix(glm::mat4 projection) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 viewProjection) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse, glm::mat4 projectionInverseTranspose) {
            // Implementation here
        }

        void Camera::setViewProjectionMatrix(glm::mat4 view, glm::mat4 projection, glm::mat4 viewProjection, glm::mat4 viewProjectionInverse, glm::mat4 viewInverse, glm::mat4 projectionInverse, glm::mat4 projectionInverseTranspose, glm::mat4 viewInverseTranspose) {
            // Implementation here
        }

        glm::mat4 Camera::getViewMatrix()
        {
            return this->view;
        }

        glm::vec3 Camera::getView()
        {
            return this->front;
        }

        glm::mat4 Camera::getProjectionMatrix()
        {
            return this->projection;
        }
        void Camera::getProjection(float& fov, float& aspect, float& nearP, float& farP)
        {
            // Implementation here
            fov = this->fov;
            aspect = this->aspect;
            nearP = this->nearP;
            farP = this->farP;
        }
    }
}