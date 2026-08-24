#include "moveDemo.h"

using namespace vkengine;

// ---------------------------------------------------------------------------
// 자유 카메라 튜닝 값
// ---------------------------------------------------------------------------
static constexpr cFloat CAMERA_BASE_SPEED = 50.0f; // 기본 이동 속도 (units/sec)
static constexpr cFloat CAMERA_BOOST_MUL = 6.0f;  // Shift 부스트 배율
static constexpr cFloat CAMERA_SLOW_MUL = 0.2f;   // Alt 감속 배율

// Camera2 내부 sensitivity(0.001f)에 곱해지는 추가 배율.
// mouseDelta 는 (이전 좌표 - 현재 좌표) 이므로 부호를 뒤집어야
// "마우스 오른쪽 -> 시야 오른쪽", "마우스 아래 -> 시야 아래" 가 된다.
static constexpr cFloat LOOK_SENSITIVITY = 2.0f;
static constexpr cFloat INVERT_X = -1.0f;
static constexpr cFloat INVERT_Y = -1.0f;

// 시작 위치 / 시작 각도 (sponza 기준 실내 눈높이)
#define CAMERA_START_POSITION cVec3(0.0f, 10.0f, 0.0f)
static constexpr cFloat CAMERA_START_YAW = 0.0f;
static constexpr cFloat CAMERA_START_PITCH = 0.0f;

// 이 sponza 는 센티미터 스케일 모델이다. 실제 바운딩 박스가
// 3721 x 1556 x 2288 (대각선 약 4637) 이라 엔진 기본값
// near 0.1 / far 1000 으로는 씬 뒤쪽이 far 평면에 잘려 나간다.
// far 만 늘리면 far/near 비가 커져 원거리 Z-fighting 이 생기므로 near 도 함께 올린다.
static constexpr cFloat CAMERA_NEAR_PLANE = 1.0f;
static constexpr cFloat CAMERA_FAR_PLANE = 6000.0f;

namespace vkengine
{
    DemoApplication::DemoApplication(cString root_path) : Application(root_path)
    {
        this->setupCamera();
    }

    DemoApplication::DemoApplication(const ApplicationConfig &config, cString root_path)
        : Application(config, root_path)
    {
        this->setupCamera();
    }

    void DemoApplication::setupCamera()
    {
        camera->setCameraType(object::CameraType::FreeCamera);
        camera->setPos(CAMERA_START_POSITION);
        camera->setYaw(CAMERA_START_YAW);
        camera->setPitch(CAMERA_START_PITCH);

        // yaw/pitch 로부터 dir/right 를 다시 만들기 위해 0 회전을 한 번 적용한다.
        camera->RotateDeltaRotation(cVec3(0.0f));

        // aspect 는 Application::initializeVulkanResources() 에서 이미 설정되어 있다.
        camera->setPerspectiveProjection(camera->getFov(),
                                         camera->getAspect(),
                                         CAMERA_NEAR_PLANE,
                                         CAMERA_FAR_PLANE);
        camera->update();
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
            window->getmouseState().mouseDelta = cVec3(0.0f);
            window->pollEvents();

            // NEW: Calculate delta time for smooth animation
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Clamp delta time to prevent large jumps (e.g., when debugging)
            deltaTime = std::min(deltaTime, 0.033f); // Max 33ms (30 FPS minimum)

            updateGui();

            // ---- free camera ----
            this->updateCameraLook();
            this->updateCameraMove(deltaTime);
            camera->update();

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

            this->updateLightSpaceMatrix();

            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;
        }
        this->cxt->waitIdle();
    }

    void DemoApplication::updateCameraLook()
    {
        auto &mouse = window->getmouseState();

        // mouseDelta 는 오른쪽 버튼을 누르고 드래그할 때만 채워진다 (VKwindow.cpp 참고).
        if (mouse.mouseDelta.x == 0.0f && mouse.mouseDelta.y == 0.0f)
            return;

        // Camera2 내부에서 force * sensitivity(0.001f) 로 라디안 변환된다.
        camera->RotateDeltaRotation(
            cVec3(mouse.mouseDelta.x * LOOK_SENSITIVITY * INVERT_X,
                  mouse.mouseDelta.y * LOOK_SENSITIVITY * INVERT_Y,
                  0.0f));
    }

    void DemoApplication::updateCameraMove(cFloat deltaTime)
    {
        const auto &keys = window->getKeyBoardState();

        // 속도 모디파이어
        cFloat speed = CAMERA_BASE_SPEED;
        if (keys.m_keyPressed[GLFW_KEY_LEFT_SHIFT])
            speed *= CAMERA_BOOST_MUL;
        if (keys.m_keyPressed[GLFW_KEY_LEFT_ALT])
            speed *= CAMERA_SLOW_MUL;
        camera->setSpeed(speed);

        // Camera2 의 이동 헬퍼는 dir/right/up 을 그대로 사용한다.
        //  MoveForward(+) : 시선 방향 전진,  MoveForward(-) : 후진
        //  MoveRight(+)   : 화면 오른쪽,     MoveRight(-)   : 화면 왼쪽
        //  MoveUp(+)      : 월드 +Y,         MoveUp(-)      : 월드 -Y
        if (keys.m_keyPressed[GLFW_KEY_W])
            camera->MoveForward(deltaTime);
        if (keys.m_keyPressed[GLFW_KEY_S])
            camera->MoveForward(-deltaTime);
        if (keys.m_keyPressed[GLFW_KEY_D])
            camera->MoveRight(deltaTime);
        if (keys.m_keyPressed[GLFW_KEY_A])
            camera->MoveRight(-deltaTime);
        if (keys.m_keyPressed[GLFW_KEY_E])
            camera->MoveUp(deltaTime);
        if (keys.m_keyPressed[GLFW_KEY_Q])
            camera->MoveUp(-deltaTime);  
    }

    void DemoApplication::updateLightSpaceMatrix()
    {
        if (models.empty())
            return;

        cMat4 lightView = glm::lookAt(cVec3(0.0f), -Renderer.getSceneDataUBO().directionalLightDir, cVec3(0.0f, 0.0f, 1.0f));

        // Transform the first model's bounding box to find the initial light bounding box
        // 초기 빛 경계 상자를 찾기 위해 첫 번째 모델의 경계 상자를 변환합니다.
        cVec3 firstMin = cVec3(models[0].ModelResource().modelMatrix * cVec4(models[0].BoundingBoxMin(), 1.0f));
        cVec3 firstMax = cVec3(models[0].ModelResource().modelMatrix * cVec4(models[0].BoundingBoxMax(), 1.0f));

        // Ensure min is actually smaller than max for each component
        // 각 구성 요소에 대해 최소값이 실제로 최대값보다 작은지 확인합니다.
        cVec3 min_ = glm::min(firstMin, firstMax);
        cVec3 max_ = glm::max(firstMin, firstMax);

        // Iterate through all models to find the combined bounding box
        // 모든 모델을 반복하여 결합된 경계 상자를 찾습니다.
        for (uint32_t i = 1; i < models.size(); i++)
        {
            cVec3 modelMin = cVec3(models[i].ModelResource().modelMatrix * cVec4(models[i].BoundingBoxMin(), 1.0f));
            cVec3 modelMax = cVec3(models[i].ModelResource().modelMatrix * cVec4(models[i].BoundingBoxMax(), 1.0f));

            cVec3 transformedMin = glm::min(modelMin, modelMax);
            cVec3 transformedMax = glm::max(modelMin, modelMax);

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
}
