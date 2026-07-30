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
        cfg.objectCount = static_cast<cUint32_t>(models.size() - 1); // Box 제외
        cfg.objectRadius = sphereRadius;
        cfg.worldBounds = boxBounds;

        physics = std::make_unique<CalculatePhysicsSimulation>(cfg);
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
            this->Renderer.addDebugAABB(models[0].Meshes()[0].worldBounds, cVec3(1,1,0)); // 박스 경계

            if (physics)
            {
                physics->update(deltaTime);

                for (cUint32_t i = 0; i < physics->ObjectCount(); ++i)
                {
                    VKModel &sphere = models[i + 1];
                    cMat4 m = glm::translate(cMat4(1.0f), physics->Position(i));
                    m = glm::scale(m, cVec3(sphereScale));
                    sphere.ModelResource().modelMatrix = m;
                    sphere.Visible() = physics->IsVisible(i); // 패턴B로 사라진 프레임엔 렌더 스킵

                    if (physics->IsVisible(i))
                    {
                        AABB sphereBox = AABB::fromCenterExtents(physics->Position(i), cVec3(physics->Radius(i)));
                        this->Renderer.addDebugAABB(sphereBox, cVec3(0, 1, 0)); // 충돌 판정에 실제 쓰이는 AABB
                    }
                }
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