#include "PhysicalSimulation.h"

namespace vkengine
{
    PhysicalSimulation::PhysicalSimulation(cString root_path) : Application(root_path)
    {
        initializePhysics();
    }
    PhysicalSimulation::PhysicalSimulation(const ApplicationConfig &config, cString root_path) : Application(config, root_path)
    {
        initializePhysics();
    }
    PhysicalSimulation::~PhysicalSimulation()
    {
    }

    void PhysicalSimulation::initializePhysics()
    {
        // models[0] = Box(컨테이너), models[1..] = Sphere
        if (models.size() < 2)
            return;

        // Box의 worldBounds — Application::run()이 매 프레임 갱신하지만, update()가 run()보다 먼저 실행되므로
        // 첫 프레임 물리 계산을 위해 여기서 한 번 미리 계산해둠 (docs/physics-plan.md와 동일한 주의사항)
        models[0].Meshes()[0].updateWorldBounds(models[0].ModelResource().modelMatrix);
        AABB boxBounds = models[0].Meshes()[0].worldBounds;

        // Sphere의 반지름 — 로컬 바운드(minBounds/maxBounds)에 스케일을 곱해서 산출
        const cVec3 &sMin = models[1].Meshes()[0].minBounds;
        const cVec3 &sMax = models[1].Meshes()[0].maxBounds;
        cFloat sphereRadius = glm::length(sMax - sMin) * 0.5f * sphereScale;

        CalculatePhysicsSimulation::Config cfg;
        // cfg.objectCount = static_cast<cUint32_t>(models.size() - 1); // Box 제외
        cfg.objectCount = this->instanceCount; // Sphere 개수
        cfg.objectRadius = sphereRadius;
        cfg.worldBounds = boxBounds;

        physics = std::make_unique<CalculatePhysicsSimulation>(cfg);

        this->Renderer.createInstanceBuffers(this->instanceCount);
        
        this->sphereInstanceData.reserve(instanceCount);
        this->sphereInstanceData.resize(instanceCount);


    }
    void PhysicalSimulation::update()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.016f; // Default to ~60 FPS

        while (!window->shouldClose())
        {
            window->getmouseState().mouseDelta = cVec3(0.0f); // ← 추가
            window->pollEvents();

            // Calculate delta time for camera updates
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            deltaTime = std::min(deltaTime, 0.016f); // Clamp delta time to prevent large jumps

            // camera update
            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_W])
            {
                this->camera->MoveForward(deltaTime);
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_S])
            {
                this->camera->MoveForward(-deltaTime);
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_A])
            {
                this->camera->MoveRight(-deltaTime);
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_D])
            {
                this->camera->MoveRight(deltaTime);
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_Q])
            {
                this->camera->MoveUp(-deltaTime);
            }

            if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_E])
            {
                this->camera->MoveUp(deltaTime);
            }

            vkengine::platform::MouseState &mouseState = this->window->getmouseState();

            this->camera->RotateDeltaRotation(mouseState.mouseDelta); // 마우스 이동에 따른 회전, 감도 조절
            this->camera->update();

            this->Renderer.getSceneDataUBO().projection = this->camera->getProjectionMatrix();
            this->Renderer.getSceneDataUBO().view = this->camera->getViewMatrix();
            this->Renderer.getSceneDataUBO().cameraPos = camera->getPos();

            this->Renderer.clearDebugLines();
            this->Renderer.addDebugAABB(models[0].Meshes()[0].worldBounds, cVec3(1, 1, 0)); // 박스 경계

            if (physics)
            {
                physics->update(deltaTime);
                sphereInstanceData.clear();
                for (cUint32_t i = 0; i < physics->ObjectCount(); ++i)
                {
                    cFloat r = physics->Radius(i);

                    cMat4 m = glm::translate(cMat4(1.0f), physics->Position(i));
                    m = glm::scale(m, cVec3(r * 2.0f));
                    cFloat visible = physics->IsVisible(i) ? 1.0f : 0.0f; // 패턴B로 재생성 대기 중이면 숨김
                    sphereInstanceData.push_back(InstanceData(m));
                }
                // 테스트를 위해 지우는 것은 일부러 제거
                this->Renderer.getSphereinstanceBuffer(currentFrame).updateData(sphereInstanceData.data(), sizeof(InstanceData) * sphereInstanceData.size(), 0);
            }

            this->updateGui();

            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;
        }
        this->cxt->waitIdle();
    }
}