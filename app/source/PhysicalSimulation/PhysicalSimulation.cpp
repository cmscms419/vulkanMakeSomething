#include "PhysicalSimulation.h"
#include "log.h"

using namespace vkengine::Log;

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
        // Box의 worldBounds — Application::run()이 매 프레임 갱신하지만, update()가 run()보다 먼저 실행되므로
        // 첫 프레임 물리 계산을 위해 여기서 한 번 미리 계산해둠 (docs/physics-plan.md와 동일한 주의사항)
        // models[0].Meshes()[0].updateWorldBounds(models[0].ModelResource().modelMatrix);

        // Box의 AABB worldBounds — Box 모델의 minBounds/maxBounds를 world 좌표계로 변환한 값
        // obj를 가져오지 않고 임의의 값으로 넣음
        cVec3 boxMin = cVec3(-100.0f, -100.0f, -100.0f);
        cVec3 boxMax = cVec3(100.0f, 100.0f, 100.0f);
        AABB boxBounds = AABB(boxMin, boxMax);

        // Sphere 모델 찾기

        cUint32_t index = 0;

        for (auto &model : models)
        {
            if (model.Name() == "Sphere")
            {
                model.Visible() = false; // Sphere 모델은 물리 계산용으로만 사용, 렌더링은 인스턴스로 처리

                // Sphere의 반지름 — 로컬 바운드(minBounds/maxBounds)에 스케일을 곱해서 산출
                const cVec3 &sMin = model.Meshes()[0].minBounds;
                const cVec3 &sMax = model.Meshes()[0].maxBounds;
                cFloat sphereRadius = glm::length(sMax - sMin) * 0.5f;

                CalculatePhysicsSimulation::Config cfg;

                // cfg.objectCount = static_cast<cUint32_t>(models.size() - 1); // Box 제외
                cfg.objectCount = maxInstanceCount; // Sphere 개수
                cfg.objectRadius = sphereRadius;
                cfg.worldBounds = boxBounds;
                cfg.restitution = 1.0f; // 완전 탄성 충돌
                cfg.minSpeed = 0.0f;
                cfg.maxSpeed = 20.0f;
                cfg.respawnInvisibleFrames = 1;
                cfg.collisionCooldownFrames = 3;

                physics = std::make_unique<CalculatePhysicsSimulation>(cfg);

                this->createInstanceBuffers(cfg.objectCount, index); // Sphere 모델 인덱스 전달

                this->sphereInstanceData.reserve(cfg.objectCount);
                this->sphereInstanceData.resize(cfg.objectCount);
                break;
            }
            index++;
        }
    }

    void PhysicalSimulation::update()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.016f;                  // Default to ~60 FPS
        this->camera->setSpeed(this->speedCamera); // Camera 이동 속도 설정

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

            // 추가해야 할것
            // 랜더링 하는 것과 안하는 것
            // max : 최대 개수
            // instanceCount : 현재 개수
            // sphereInstanceData : 현재 랜더링 되어야 하는 개수
            if (physics)
            {
                this->updateGui();
                physics->update(deltaTime);
            }

            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;
        }
        this->cxt->waitIdle();
    }

    void PhysicalSimulation::run()
    {
        _VK_CHECK_RESULT_(vkWaitForFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame]));

        Renderer.update(*camera.get(), currentFrame, (float)glfwGetTime() * 0.5f);
        // Perform frustum culling on all models
        guiRenderer.update();

        if (this->physics)
        {
            physics->updateInstanceData(sphereInstanceData);
            updateInstanceData(sphereInstanceData.size());
        }

        uint32_t imageIndex{0};
        VkResult result = this->swapChain->acquireNextImage(presentSemaphores[currentSemaphore], imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || this->framebufferResized)
        {
            this->framebufferResized = false;
            recreateSwapchain();
            return;
        }
        else if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
        {
            EXIT_TO_LOGGER("Could not acquire the next swap chain image!");
        }

        // Use currentFrame index (CPU-side command buffer)
        VKCommandBufferHander &cmd = this->commandBuffers[currentFrame];

        // Begin command buffer
        vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
        VkCommandBufferBeginInfo cmdBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));
        {
            VkViewport viewport{0.0f, 0.0f, (float)this->extent.width, (float)this->extent.height, 0.0f, 1.0f};
            VkRect2D scissor{0, 0, this->extent.width, this->extent.height};

            // Draw models
            this->Renderer.rendering(cmd.getCommandBuffer(), currentFrame, imageIndex, this->models, viewport, scissor);

            // Draw GUI (overwrite to swapchain image)
            this->guiRenderer.draw(cmd.getCommandBuffer(), this->swapChain->getSwapChainImageView(imageIndex), viewport);

            // Transition swapchain image to present layout
            this->swapChain->transitionTo(cmd.getCommandBuffer(), imageIndex);
        }
        _VK_CHECK_RESULT_(vkEndCommandBuffer(cmd.getCommandBuffer())); // End command buffer

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // 비교: 마지막으로 실행되는 셰이더가 Compute라면 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT

        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.pCommandBuffers = &cmd.getCommandBuffer();
        submitInfo.commandBufferCount = 1;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.pWaitSemaphores = &this->presentSemaphores[currentSemaphore];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &this->renderSemaphores[currentSemaphore];
        submitInfo.signalSemaphoreCount = 1;
        _VK_CHECK_RESULT_(vkQueueSubmit(cmd.getQueue(), 1, &submitInfo, this->inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &this->renderSemaphores[currentSemaphore];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &this->swapChain->getSwapChain();
        presentInfo.pImageIndices = &imageIndex;
        VkResult presentResult = vkQueuePresentKHR(this->cxt->grapicsQueue(), &presentInfo);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        {
            this->framebufferResized = false;
            recreateSwapchain();
        }
        else if (presentResult != VK_SUCCESS)
        {
            EXIT_TO_LOGGER("Failed to present swapchain image!");
        }
    }

    void PhysicalSimulation::updateGui()
    {
        if (physics)
        {
            // physical simulation 전용 GUI 업데이트
            static float scale = 1.4f;
            ImGuiIO &io = ImGui::GetIO();

            io.DisplaySize = ImVec2(float(extent.width), float(extent.height));
            platform::MouseState &mousetState = this->window->getmouseState(); // Update mouse state

            // Always pass mouse input to ImGui - let ImGui decide if it wants to capture it
            io.MousePos = ImVec2(mousetState.position.x, mousetState.position.y);
            io.MouseDown[0] = mousetState.buttons.left;
            io.MouseDown[1] = mousetState.buttons.right;
            io.MouseDown[2] = mousetState.buttons.middle;

            ImGui::NewFrame();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
            ImGui::SetNextWindowPos(ImVec2(10 * scale, 10 * scale), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::Begin("Physical Simulation Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            CalculatePhysicsSimulation::Config cfg = physics->getConfig();

            // cFloat speed = physics->getSimulationSpeed();
            // if (ImGui::SliderFloat("Simulation Speed", &speed, 0.0f, 10.0f))
            // {
            //     physics->setSimulationSpeed(speed);
            // }

            cFloat radius = physics->Radius(); // 모든 Sphere가 동일한 반지름을 가지므로 첫 번째 Sphere의 반지름을 사용

            if (ImGui::SliderFloat("Sphere Radius", &radius, 0.1f, 5.0f))
            {
                cfg.objectRadius = radius;
            }

            cInt objectCount = static_cast<cInt>(physics->ObjectCount());

            if (ImGui::SliderInt("instance Count", &objectCount, 1, static_cast<int>(maxInstanceCount)))
            {
                cfg.objectCount = static_cast<cUint32_t>(objectCount);
            }

            cFloat maxSpeed = cfg.maxSpeed;

            if (ImGui::SliderFloat("instance max Speed", &maxSpeed, cfg.minSpeed, 20.0f))
            {
                cfg.maxSpeed = maxSpeed;
            }

            static cVec3 lightColor = cVec3(1.0f);
            static cFloat lightIntensity = 28.454f;
            ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 100.0f);
            this->Renderer.getSceneDataUBO().directionalLightColor = lightIntensity * lightColor;

            // TODO: IS there a way to determine directionalLightColor from time of day? bright morning sun
            // to noon white light to golden sunset color.

            static float elevation = 65.2f; // Elevation angle (up/down) in degrees
            static float azimuth = -143.8f; // Azimuth angle (left/right) in degrees

            ImGui::SliderFloat("Light Elevation", &elevation, -90.0f, 90.0f, "%.1f°");
            ImGui::SliderFloat("Light Azimuth", &azimuth, -180.0f, 180.0f, "%.1f°");

            // Convert to radians
            float elev_rad = glm::radians(elevation);
            float azim_rad = glm::radians(azimuth);

            // Calculate direction using standard spherical coordinates
            glm::vec3 lightDir;
            lightDir.x = cos(elev_rad) * sin(azim_rad);
            lightDir.y = sin(elev_rad);
            lightDir.z = cos(elev_rad) * cos(azim_rad);

            // Set the light direction (already normalized from spherical coordinates)
            this->Renderer.getSceneDataUBO().directionalLightDir = lightDir;

            // Display current light direction for debugging
            ImGui::Text("Light Dir: (%.2f, %.2f, %.2f)",
                        this->Renderer.getSceneDataUBO().directionalLightDir.x,
                        this->Renderer.getSceneDataUBO().directionalLightDir.y,
                        this->Renderer.getSceneDataUBO().directionalLightDir.z);

            ImGui::Text("Rendering Instance Count %d", static_cast<cInt>(this->sphereInstanceData.size()));

            ImGui::End();
            ImGui::PopStyleVar();

            this->physics->updateConfig(cfg);

            ImGui::Render();
        }
    }

    void PhysicalSimulation::createInstanceBuffers(cUint32_t maxInstanceCount, cUint32_t modelIndex)
    {
        if (this->models.empty() || modelIndex >= this->models.size())
        {
            EXIT_TO_LOGGER("Models are not set or invalid model index. Cannot create instance buffers.");
        }

        this->Renderer.createInstanceBuffers(maxInstanceCount);

        DrawModelResource resource = this->models[modelIndex].ModelResource();

        this->Renderer.getDrawInstanceResource().materialIndex = this->models[modelIndex].Meshes()[0].materialIndex;
        this->Renderer.getDrawInstanceResource().modelMatrix = glm::scale(cMat4(1.0f), cVec3(this->physics->Radius()));
        std::fill(std::begin(this->Renderer.getDrawInstanceResource().coeffs), std::end(this->Renderer.getDrawInstanceResource().coeffs), 0.0f);
        
        this->Renderer.getDrawInstanceResource().coeffs[0] = resource.coeffs[0];
        this->Renderer.getDrawInstanceResource().coeffs[1] = resource.coeffs[1];
        this->Renderer.getDrawInstanceResource().coeffs[2] = resource.coeffs[2];
        this->Renderer.getDrawInstanceResource().coeffs[3] = resource.coeffs[3];
        this->Renderer.getDrawInstanceResource().coeffs[4] = resource.coeffs[4];
        this->Renderer.getDrawInstanceResource().coeffs[5] = resource.coeffs[5];
    }

    void PhysicalSimulation::updateInstanceData(cUint32_t currentInstanceCount)
    {
        this->Renderer.getSphereinstanceBuffer(currentFrame).updateData(sphereInstanceData.data(), sizeof(InstanceData) * currentInstanceCount, 0);
        this->Renderer.updateInstance(currentInstanceCount);

        cMat4 modelMatrix = glm::scale(cMat4(1.0f), cVec3(this->physics->Radius()));

        this->Renderer.getDrawInstanceResource().modelMatrix = modelMatrix; // Transpose of the inverse for normal transformation
    }
}