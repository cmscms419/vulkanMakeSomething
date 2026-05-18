#include "moveDemo.h"
#define CAMERA_POSITION glm::vec3(0.0f, 2.0f, 3.0f) // 카메라 초기 위치

using namespace vkengine;

constexpr float MOVE_SPEED = 3.0f;

static constexpr float GRAVITY = 9.8f;
static constexpr float JUMP_SPEED = 5.0f;
static constexpr float TERMINAL_VEL = -20.0f;

static constexpr float STEP_HEIGHT = 0.35f;
static constexpr float CHAR_HEIGHT = 1.0f;
static constexpr float CHAR_RADIUS = 0.3f;

static constexpr cFloat ROTATION_SPEED = 10.0f;

namespace vkengine
{
    DemoApplication::DemoApplication(cString root_path) : Application(root_path)
    {
        this->velocity = cVec3(0.0f);
        this->isGrounded = false;
        this->isMoving = false;
        this->isSpeedUp = false;
        this->currentFacingYaw = glm::radians(-90.0f);

        // createDefault() 기준 캐릭터 위치
        glm::vec3 characterPos = this->models[0].ModelResource().modelMatrix[3]; // 모델 행렬의 4번째 열에서 위치 추출
        glm::vec3 cameraPos = characterPos + CAMERA_POSITION; // 카메라 위치를 캐릭터 뒤로 설정

        // 카메라 → 캐릭터 방향 계산
        glm::vec3 toTarget = glm::normalize(characterPos - cameraPos);
        float desiredYaw = std::atan2(toTarget.x, -toTarget.z);
        float desiredPitch = std::asin(-toTarget.y);

        // sensitivity = 0.001f 이므로 역산
        camera->RotateDeltaRotation(glm::vec3(desiredYaw / 0.001f, desiredPitch / 0.001f, 0.0f));
    }

    DemoApplication::~DemoApplication()
    {
    }

    void DemoApplication::update()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.016f; // Default to ~60 FPS

        while (!window->shouldClose())
        {
            window->getmouseState().mouseDelta = cVec3(0.0f); // ← 추가
            window->pollEvents();

            // NEW: Calculate delta time for smooth animation
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Clamp delta time to prevent large jumps (e.g., when debugging)
            deltaTime = std::min(deltaTime, 0.033f); // Max 33ms (30 FPS minimum)

            updateGui();

            // charater move
            cVec3 characterPosition = models[0].ModelResource().modelMatrix[3]; // 모델 행렬의 4번째 열에서 위치 추출
            cVec3 moveInput = cVec3(0.0f);
            cVec3 forward = normalize(cVec3(camera->getDir().x, 0, camera->getDir().z));
            cVec3 right = normalize(glm::cross(forward, cVec3(0, 1, 0)));

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_W])
                moveInput -= forward * deltaTime; // 앞으로 이동

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_S])
                moveInput += forward * deltaTime; // 뒤로 이동

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_A])
                moveInput += right * deltaTime; // 왼쪽으로 이동

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_D])
                moveInput -= right * deltaTime; // 오른쪽으로 이동
            
            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_LEFT_SHIFT])
                this->isSpeedUp = true;
            else
                this->isSpeedUp = false;

            if (glm::length(moveInput) > 0.0f)
            {
                moveInput = glm::normalize(moveInput);
                velocity.x = moveInput.x * MOVE_SPEED;
                velocity.z = moveInput.z * MOVE_SPEED;

                if (!isMoving)
                {
                    isMoving = true;
                    models[0].setAnimationIndex(1); // 걷기 애니메이션으로 전환
                }

                if (isSpeedUp)
                {
                    velocity.x *= 2.0f; // Shift 키로 이동 속도 증가
                    velocity.z *= 2.0f; // Shift 키로 이동 속도 증가
                }
            }
            else
            {
                velocity.x = 0.0f;
                velocity.z = 0.0f;

                if (isMoving)
                {
                    isMoving = false;
                    models[0].setAnimationIndex(0); // 대기 애니메이션으로 전환
                }
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_SPACE] && isGrounded)
            {
                velocity.y = JUMP_SPEED; // 점프 속도 설정
                isGrounded = false;      // 공중에 떠 있는 상태로 전환
            }

            // Apply gravity
            if (!isGrounded)
            {
                velocity.y -= GRAVITY * deltaTime;               // 중력 가속도 적용
                velocity.y = std::max(velocity.y, TERMINAL_VEL); // 최대 낙하 속도 제한
            }

            characterPosition += velocity * deltaTime;

            cFloat groundY = this->FindYGround(characterPosition);
            // this->resolveWallCollisions(characterPosition);
            cBool groundFound = groundY > -FLT_MAX / 2.0f;

            if (groundFound && characterPosition.y <= groundY)
            {
                characterPosition.y = groundY;
                if (velocity.y < 0.0f)
                    velocity.y = 0.0f;
                isGrounded = true;
            }
            else
            {
                isGrounded = false;
            }

            // models[0].ModelResource().modelMatrix[3] = cVec4(characterPosition, 1.0f); // 모델 행렬의 4번째 열에 위치 업데이트

            // cFloat currentFacingYaw = std::atan2(velocity.x, velocity.z); // 카메라 방향에 맞춰 캐릭터 회전

            if (glm::length(cVec2(velocity.x, velocity.z)) > 0.1f)
            {
                float targetYaw = std::atan2(velocity.x, velocity.z);

                // 최단 경로 각도 차이 계산 (-π ~ π 범위로 정규화)
                float diff = targetYaw - currentFacingYaw;
                diff = std::atan2(std::sin(diff), std::cos(diff));

                currentFacingYaw += diff * ROTATION_SPEED * deltaTime;
            }

            cMat4 newMatrix = glm::translate(cMat4(1.0f), characterPosition);     // T
            newMatrix = glm::scale(newMatrix, cVec3(0.012f));                     // T * S
            newMatrix = glm::rotate(newMatrix, currentFacingYaw, cVec3(0, 1, 0)); // T * S * R
            models[0].ModelResource().modelMatrix = newMatrix;

            // this->camera->setPos(characterPosition + CAMERA_POSITION); // 카메라 위치를 캐릭터 뒤로 설정
            // this->camera->update();

            auto &mouse = window->getmouseState();

            camera->updateOrbit(
                characterPosition + glm::vec3(0, 0.8f, 0), // 캐릭터 상체
                mouse.mouseDelta.x * 0.01f,                // sensitivity
                mouse.mouseDelta.y * 0.01f);               // sensitivity

            this->Renderer.getSceneDataUBO().projection = this->camera->getProjectionMatrix();
            this->Renderer.getSceneDataUBO().view = this->camera->getViewMatrix();
            this->Renderer.getSceneDataUBO().cameraPos = camera->getPos();

            for (auto &model : models)
            {
                if (model.hasAnimations())
                {
                    model.updateAnimation(deltaTime);
                }
            }

            if (models.size() > 0)
            {
                cMat4 lightView = glm::lookAt(cVec3(0.0f), -Renderer.getSceneDataUBO().directionalLightDir, cVec3(0.0f, 0.0f, 1.0f));

                // Transform the first model's bounding box to find the initial light bounding box
                // 초기 빛 경계 상자를 찾기 위해 첫 번째 모델의 경계 상자를 변환합니다.
                cVec3 firstMin =
                    cVec3(models[0].ModelResource().modelMatrix * cVec4(models[0].BoundingBoxMin(), 1.0f));
                cVec3 firstMax =
                    cVec3(models[0].ModelResource().modelMatrix * cVec4(models[0].BoundingBoxMax(), 1.0f));

                // Ensure min is actually smaller than max for each component
                // 각 구성 요소에 대해 최소값이 실제로 최대값보다 작은지 확인합니다.
                cVec3 min_ = glm::min(firstMin, firstMax);
                cVec3 max_ = glm::max(firstMin, firstMax);

                // Iterate through all models to find the combined bounding box
                // 모든 모델을 반복하여 결합된 경계 상자를 찾습니다.
                for (uint32_t i = 1; i < models.size(); i++)
                {
                    // Transform this model's bounding box to world space
                    // 이 모델의 경계 상자를 월드 공간으로 변환합니다.
                    cVec3 modelMin = cVec3(models[i].ModelResource().modelMatrix * cVec4(models[i].BoundingBoxMin(), 1.0f));
                    cVec3 modelMax = cVec3(models[i].ModelResource().modelMatrix * cVec4(models[i].BoundingBoxMax(), 1.0f));

                    // Ensure proper min/max ordering
                    // 올바른 최소/최대 순서 보장
                    cVec3 transformedMin = glm::min(modelMin, modelMax);
                    cVec3 transformedMax = glm::max(modelMin, modelMax);

                    // Expand the overall bounding box
                    // 전체 경계 상자 확장
                    min_ = glm::min(min_, transformedMin);
                    max_ = glm::max(max_, transformedMax);
                }

                cVec3 corners[] = {
                    cVec3(min_.x, min_.y, min_.z),
                    cVec3(min_.x, max_.y, min_.z),
                    cVec3(min_.x, min_.y, max_.z),
                    cVec3(min_.x, max_.y, max_.z),
                    cVec3(max_.x, min_.y, min_.z),
                    cVec3(max_.x, max_.y, min_.z),
                    cVec3(max_.x, min_.y, max_.z),
                    cVec3(max_.x, max_.y, max_.z),
                };
                cVec3 vmin(std::numeric_limits<float>::max());
                cVec3 vmax(std::numeric_limits<float>::lowest());
                for (size_t i = 0; i != 8; i++)
                {
                    auto temp = cVec3(lightView * cVec4(corners[i], 1.0f));
                    vmin = glm::min(vmin, temp);
                    vmax = glm::max(vmax, temp);
                }
                min_ = vmin;
                max_ = vmax;
                cMat4 lightProjection = glm::orthoLH_ZO(min_.x, max_.x, min_.y, max_.y, max_.z, min_.z); // 마지막 Max, Min 순서 주의
                Renderer.getSceneDataUBO().lightSpaceMatrix = lightProjection * lightView;

                // Modifed "Vulkan 3D Graphics Rendering Cookbook - 2nd Edition Build Status"
                // https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook-Second-Edition
            }

            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;
        }
        this->cxt->waitIdle();
    }

    cFloat DemoApplication::FindYGround(const cVec3 &position)
    {
        if (models.size() < 2)
            return -FLT_MAX;

        float groundY = -FLT_MAX;
        float searchOriginY = position.y + 0.05f; // 발보다 살짝 위에서 아래로 탐색

        for (const auto &mesh : models[1].Meshes())
        {
            const AABB &bb = mesh.worldBounds;

            // 유효하지 않은 bounds 스킵
            if (bb.max == bb.min)
                continue;

            // XZ 범위 포함 여부
            if (position.x < bb.min.x || position.x > bb.max.x)
                continue;
            if (position.z < bb.min.z || position.z > bb.max.z)
                continue;

            float surfaceY = bb.max.y;

            // 계단 오르기: STEP_HEIGHT 이내의 표면만 허용
            if (surfaceY > searchOriginY + 0.5f) // 작은 허용 오차 추가
                continue;

            groundY = std::max(groundY, surfaceY);
        }

        return groundY;
    }

    void DemoApplication::resolveWallCollisions(cVec3 &position)
    {
        for (int iter = 0; iter < 2; ++iter)
        { // 2회 반복으로 안정성 확보
            for (const auto &mesh : models[1].Meshes())
            {
                const AABB &bb = mesh.worldBounds;

                float bbH = bb.max.y - bb.min.y;

                // 얇은 수평면(바닥/천장) 스킵 → 벽만 처리
                if (bbH < 0.15f)
                    continue;

                // 캐릭터 높이와 수직 겹침 확인
                if (bb.max.y < position.y + 0.1f)
                    continue; // 발 아래
                if (bb.min.y > position.y + CHAR_HEIGHT)
                    continue; // 머리 위

                // 캐릭터 XZ AABB
                float cMinX = position.x - CHAR_RADIUS, cMaxX = position.x + CHAR_RADIUS;
                float cMinZ = position.z - CHAR_RADIUS, cMaxZ = position.z + CHAR_RADIUS;

                // XZ 겹침 없으면 스킵
                if (cMaxX <= bb.min.x || cMinX >= bb.max.x)
                    continue;
                if (cMaxZ <= bb.min.z || cMinZ >= bb.max.z)
                    continue;

                // 각 축 침투량
                float oxR = cMaxX - bb.min.x, oxL = bb.max.x - cMinX;
                float ozF = cMaxZ - bb.min.z, ozB = bb.max.z - cMinZ;
                float ox = std::min(oxR, oxL);
                float oz = std::min(ozF, ozB);

                // 침투량이 작은 축으로 밀어냄 (슬라이딩)
                if (ox <= oz)
                    position.x += (oxR <= oxL) ? -ox : ox;
                else
                    position.z += (ozF <= ozB) ? -oz : oz;
            }
        }
    }
}