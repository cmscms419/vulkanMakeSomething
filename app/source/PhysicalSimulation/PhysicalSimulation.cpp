#include "PhysicalSimulation.h"

namespace vkengine
{
    PhysicalSimulation::PhysicalSimulation(cString root_path) : Application(root_path)
    {
    }
    PhysicalSimulation::PhysicalSimulation(const ApplicationConfig &config, cString root_path) : Application(config, root_path)
    {
    }
    PhysicalSimulation::~PhysicalSimulation()
    {
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

            this->updateGui();

            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;
        }
        this->cxt->waitIdle();
    }
}