#include "Application.h"

#include "helper.h"
#include "vkconfig.h"
#include "log.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Application.h"


namespace vkengine
{
    Application::Application(cString root_path) : Application(ApplicationConfig::createDefault(), root_path)
    {
    }

    Application::Application(const ApplicationConfig &config, cString root_path)
        : VulkanEngineWin2(true),
          RootPath{root_path.c_str()},
          shaderManager{*this->cxt,
                        RootPath + SHADER_PATH,
                        {
                            {"shadowMap", {"vertshadowMap.spv", "fragshadowMap.spv"}},
                            {"pbrdeferred", {"vertpbrdeferred.spv", "fragpbrdeferred.spv"}},
                            {"gui", {"vertimgui.spv", "fragimgui.spv"}},
                            {"instanced", {"vertphysicsInstance.spv", "fragphysicsInstance.spv"}},
                            {"sky", {"vertskybox2.spv", "fragskybox2.spv"}},
                            {"debugLine", {"vertdebugLine.spv", "fragdebugLine.spv"}},
                            {"post", {"vertpost.spv", "fragpost.spv"}},
                            {"lightdeferred", {"complightdeferred.spv"}},
                            {"ssao", {"compssao.spv"}},
                            {"ssaoBlur", {"compssaoBlur.spv"}},
                            {"cloth", {"vertcloth.spv", "fragcloth.spv"}},
                            {"clothPredict", {"compclothPredict.spv"}},
                            {"clothSolve", {"compclothSolve.spv"}},
                            {"clothFinalize", {"compclothFinalize.spv"}},
                        }},
          guiRenderer{*this->cxt, shaderManager, swapChain->getSwapChainImageFormat(), RootPath + this->AssetsPath},
          renderer(*this->cxt, shaderManager, *this->swapChain, this->kMaxFramesInFlight, RootPath + this->AssetsPath, RootPath + this->ShaderPath)
    {
        initializeVulkanResources();
        initCallbacks();
        initializeWithConfig(config);
        initializeRenderGraph(config.renderGraphFilePath);
    }

    Application::~Application()
    {
    }

    void Application::initializeWithConfig(const ApplicationConfig &config)
    {
        this->loadModels(config.models);

        this->renderer.initModelResources(
            this->models,
            this->swapChain->getSwapChainImageFormat(),
            this->cxt->getDepthStencil()->depthFormat,
            this->extent.width,
            this->extent.height);
    }

    void Application::loadModels(const std::vector<ModelConfig> &modelConfigs)
    {
        for (const auto &modelConfig : modelConfigs)
        {
            this->models.emplace_back(*cxt);
            auto &model = this->models.back();

            cString fullPath = this->RootPath + this->AssetsPath + modelConfig.filePath;
            model.loadFromModelFile(fullPath, modelConfig.isBistroObj);
            model.getName() = modelConfig.displayName;
            model.getModelResource().modelMatrix = modelConfig.transform;

            Log::printToLogger("Loaded model '%s'", modelConfig.displayName.c_str());
            Log::printToLogger("   - Meshes: %zu", model.getMeshes().size());
            Log::printToLogger("   - Materials: %zu", model.getMaterials().size());
            Log::printToLogger("   - Textures: %zu", model.getTextures().size());

            // Setup animation if model supports it
            if (model.hasAnimations() && modelConfig.autoPlayAnimation)
            {
                Log::printToLogger("   Animations: %d found", model.getAnimationCount());

                if (model.getAnimationCount() > 0)
                {
                    uint32_t animIndex = std::min(modelConfig.initialAnimationIndex,
                                                  model.getAnimationCount() - 1);
                    model.setAnimationIndex(animIndex);
                    model.setAnimationLooping(modelConfig.loopAnimation);
                    model.setAnimationSpeed(modelConfig.animationSpeed);
                    model.playAnimation();

                    Log::printToLogger("   - Started animation: '%s' (%.2f sec)",
                                    model.getAnimation()->getCurrentAnimationName().c_str(),
                                    model.getAnimation()->getDuration());
                }
            }
            else if (!model.hasAnimations())
            {
                Log::printToLogger("   Animations: None");
            }

            Log::printToLogger(""); // 빈 줄
        }
    }

    void Application::initCallbacks()
    {
        // engine에서 제공하는 카메라 컨트롤 윈도우 사용
    }
    
    void Application::initializeVulkanResources()
    {
        this->commandBuffers = this->cxt->createGraphicsCommandBufferHandles(this->kMaxFramesInFlight);
        this->camera = std::make_shared<vkengine::object::Camera2>();

        cFloat fov = camera->getFov();
        cFloat nearP = camera->getNearP();
        cFloat farP = camera->getFarP();
        cFloat aspectRatio = static_cast<cFloat>(this->extent.width) / static_cast<cFloat>(this->extent.height);
        camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);

        // engine에서 이미 fence와 semaphore를 생성하므로 생략
    }

    void Application::initializeRenderGraph(cString buildRenderGraph)
    {
        this->renderer.buildRenderGraph(buildRenderGraph);
    }

    void Application::run()
    {
        _VK_CHECK_RESULT_(vkWaitForFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame]));

        renderer.update(*camera.get(), currentFrame, (float)glfwGetTime() * 0.5f);
        renderer.updateBoneData(models, currentFrame);

        cMat4 viewProjection = this->camera->getProjectionMatrix() * this->camera->getViewMatrix();
        renderer.updateViewFrustum(viewProjection);

        for (auto &model : models)
        {
            const glm::mat4 &modelMatrix = model.getModelResource().modelMatrix;
            for (auto &mesh : model.getMeshes())
            {
                mesh.updateWorldBounds(modelMatrix);
            }
        }

        // Perform frustum culling on all models
        renderer.performFrustumCulling(models);
        guiRenderer.update(currentFrame); // 펜스 대기 뒤에 호출해야 한다 (프레임별 GUI 버퍼)

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
            Log::exitToLogger("Could not acquire the next swap chain image!");
        }

        // Use currentFrame index (CPU-side command buffer)
        CommandBufferHandle &cmd = this->commandBuffers[currentFrame];

        // Begin command buffer
        vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
        VkCommandBufferBeginInfo cmdBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));
        {
            VkViewport viewport{0.0f, 0.0f, (float)this->extent.width, (float)this->extent.height, 0.0f, 1.0f};
            VkRect2D scissor{0, 0, this->extent.width, this->extent.height};

            // Draw models
            this->renderer.render(cmd.getCommandBuffer(), currentFrame, imageIndex, this->models, viewport, scissor);

            // Draw GUI (overwrite to swapchain image)
            this->guiRenderer.draw(cmd.getCommandBuffer(), currentFrame, this->swapChain->getSwapChainImageView(imageIndex), viewport);

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
        VkResult presentResult = vkQueuePresentKHR(this->cxt->getGraphicsQueue(), &presentInfo);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            this->framebufferResized = false;
            recreateSwapchain();
        } else if (presentResult != VK_SUCCESS) {
            Log::exitToLogger("Failed to present swapchain image!");
        }
    }

    void Application::update()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.016f; // Default to ~60 FPS

        while (!window->shouldClose())
        {
            window->pollEvents();

            // NEW: Calculate delta time for smooth animation
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Clamp delta time to prevent large jumps (e.g., when debugging)
            deltaTime = std::min(deltaTime, 0.033f); // Max 33ms (30 FPS minimum)

            // camera update
            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_W])
            {
                this->camera->moveForward(deltaTime);
            }

            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_S])
            {
                this->camera->moveForward(-deltaTime);
            }

            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_A])
            {
                this->camera->moveRight(-deltaTime);
            }

            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_D])
            {
                this->camera->moveRight(deltaTime);
            }

            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_Q])
            {
                this->camera->moveUp(-deltaTime);
            }

            if (window->getKeyboardState().m_keyPressed[GLFW_KEY_E])
            {
                this->camera->moveUp(deltaTime);
            }

            updateGui();

            auto &mouse = window->getMouseState();

            this->camera->rotateDeltaRotation(mouse.mouseDelta); // 마우스 이동에 따른 회전, 감도 조절
            this->camera->update();

            this->renderer.getSceneDataUBO().projection = this->camera->getProjectionMatrix();
            this->renderer.getSceneDataUBO().view = this->camera->getViewMatrix();
            this->renderer.getSceneDataUBO().cameraPos = camera->getPos();

            for (auto &model : models)
            {
                if (model.hasAnimations())
                {
                    model.updateAnimation(deltaTime);
                }
            }

            if (models.size() > 0)
            {
                cMat4 lightView = glm::lookAt(cVec3(0.0f), -renderer.getSceneDataUBO().directionalLightDir, cVec3(0.0f, 0.0f, 1.0f));

                // Transform the first model's bounding box to find the initial light bounding box
                // 초기 빛 경계 상자를 찾기 위해 첫 번째 모델의 경계 상자를 변환합니다.
                cVec3 firstMin =
                    cVec3(models[0].getModelResource().modelMatrix * cVec4(models[0].getBoundingBoxMin(), 1.0f));
                cVec3 firstMax =
                    cVec3(models[0].getModelResource().modelMatrix * cVec4(models[0].getBoundingBoxMax(), 1.0f));

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
                    cVec3 modelMin = cVec3(models[i].getModelResource().modelMatrix * cVec4(models[i].getBoundingBoxMin(), 1.0f));
                    cVec3 modelMax = cVec3(models[i].getModelResource().modelMatrix * cVec4(models[i].getBoundingBoxMax(), 1.0f));

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
                renderer.getSceneDataUBO().lightSpaceMatrix = lightProjection * lightView;

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

    void Application::updateGui()
    {
        static float scale = 1.4f;

        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2(float(extent.width), float(extent.height));
        // io.DeltaTime = frameTimer;
        platform::MouseState &mousetState = this->window->getMouseState(); // Update mouse state

        // Always pass mouse input to ImGui - let ImGui decide if it wants to capture it
        io.MousePos = ImVec2(mousetState.position.x, mousetState.position.y);
        io.MouseDown[0] = mousetState.buttons.left;
        io.MouseDown[1] = mousetState.buttons.right;
        io.MouseDown[2] = mousetState.buttons.middle;

        // 휠을 ImGui 로 넘긴다. 넘기지 않으면 내용이 창보다 긴 패널을
        // 스크롤바를 직접 끄는 것 말고는 움직일 수 없다.
        // 카메라 줌은 handleScroll() 이 따로 하므로 그대로 같이 동작한다.
        io.MouseWheel = mousetState.scrollDelta;
        mousetState.scrollDelta = 0.0f;

        ImGui::NewFrame();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::SetNextWindowPos(ImVec2(10 * scale, 10 * scale), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Vulkan 실시간 렌더링 예제", nullptr, ImGuiWindowFlags_None);

        static cVec3 lightColor = cVec3(1.0f);
        static cFloat lightIntensity = 28.454f;
        ImGui::SliderFloat("빛 세기", &lightIntensity, 0.0f, 100.0f);
        this->renderer.getSceneDataUBO().directionalLightColor = lightIntensity * lightColor;

        // TODO: IS there a way to determine directionalLightColor from time of day? bright morning sun
        // to noon white light to golden sunset color.

        static float elevation = 65.2f; // Elevation angle (up/down) in degrees
        static float azimuth = -143.8f; // Azimuth angle (left/right) in degrees

        ImGui::SliderFloat("빛 고도", &elevation, -90.0f, 90.0f, "%.1f°");
        ImGui::SliderFloat("빛 방위", &azimuth, -180.0f, 180.0f, "%.1f°");

        // Convert to radians
        float elev_rad = glm::radians(elevation);
        float azim_rad = glm::radians(azimuth);

        // Calculate direction using standard spherical coordinates
        glm::vec3 lightDir;
        lightDir.x = cos(elev_rad) * sin(azim_rad);
        lightDir.y = sin(elev_rad);
        lightDir.z = cos(elev_rad) * cos(azim_rad);

        // Set the light direction (already normalized from spherical coordinates)
        this->renderer.getSceneDataUBO().directionalLightDir = lightDir;

        // Display current light direction for debugging
        ImGui::Text("빛 방향 : (%.2f, %.2f, %.2f)",
                    this->renderer.getSceneDataUBO().directionalLightDir.x,
                    this->renderer.getSceneDataUBO().directionalLightDir.y,
                    this->renderer.getSceneDataUBO().directionalLightDir.z);

        // Rendering Options Controls
        ImGui::Separator();
        ImGui::Text("렌더링 옵션");

        bool textureOn = this->renderer.getOptionsUBO().textureOn != 0;
        bool shadowOn = this->renderer.getOptionsUBO().shadowOn != 0;
        bool discardOn = this->renderer.getOptionsUBO().discardOn != 0;

        // NEW: Frustum Culling Controls
        ImGui::Separator();
        ImGui::Text("절두체 컬링");

        bool frustumCullingEnabled = this->renderer.isFrustumCullingEnabled();
        if (ImGui::Checkbox("절두체 컬링 사용", &frustumCullingEnabled))
        {
            this->renderer.setFrustumCullingEnabled(frustumCullingEnabled);
        }

        // Display culling statistics
        const auto &stats = this->renderer.getCullingStats();
        ImGui::Text("전체 메시 : %u", stats.totalMeshes);
        ImGui::Text("그려짐   : %u", stats.renderedMeshes);
        ImGui::Text("잘림     : %u", stats.culledMeshes);

        if (stats.totalMeshes > 0)
        {
            float cullPercent = (float)stats.culledMeshes / stats.totalMeshes * 100.0f;
            ImGui::Text("잘린 비율 : %.1f%%", cullPercent);
        }

        if (ImGui::Checkbox("텍스처", &textureOn))
        {
            this->renderer.getOptionsUBO().textureOn = textureOn ? 1 : 0;
        }
        if (ImGui::Checkbox("그림자", &shadowOn))
        {
            this->renderer.getOptionsUBO().shadowOn = shadowOn ? 1 : 0;
        }
        if (ImGui::Checkbox("알파 버림", &discardOn))
        {
            this->renderer.getOptionsUBO().discardOn = discardOn ? 1 : 0;
        }

        ImGui::Separator();

        for (uint32_t i = 0; i < models.size(); i++)
        {
            auto &m = models[i];
            char labelBuffer[128];

            // 모델 이름
            cString modelName = m.getName().empty() ? "이름 없음" : m.getName();
            snprintf(labelBuffer, sizeof(labelBuffer), "모델 %u - %s", i, modelName.c_str());

            // clean
            if (ImGui::TreeNode(labelBuffer))
            {
                // Visible 체크박스
                snprintf(labelBuffer, sizeof(labelBuffer), "보이기##%u", i);
                ImGui::Checkbox(labelBuffer, &m.isVisible());

                float *coeffs = m.getModelResource().coeffs;

                snprintf(labelBuffer, sizeof(labelBuffer), "발광##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[2], 0.0f, 10.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "금속성##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[4], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "거칠기##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[5], 0.0f, 1.0f);

                ImGui::TreePop();
            }

        }

        ImGui::End();
        ImGui::PopStyleVar();

        // Camera Control Window
        renderCameraControlWindow();

        renderHDRControlWindow();

        renderPostProcessingControlWindow();

        renderSSAOControlWindow();

        ImGui::Render();
    }

    // 애니메이션 업데이트를 위한 가상 함수, 필요에 따라 오버라이드 가능
    void Application::updateAnimation(cFloat deltaTime)
    {
    }

    void Application::renderSSAOControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(1490, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(470, 420), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("SSAO 설정"))
        {
            ImGui::End();
            return;
        }

        SSAOParamsUBO &ssao = this->renderer.getSSAOParamsUBO();

        // ssaoSampleCount == 0 이면 ssao.comp / lightdeferred.comp 양쪽에서 SSAO를 건너뛴다.
        // 토글로 껐다 켤 때 이전 샘플 수를 잃지 않도록 보관한다.
        static cInt savedSampleCount = 16;

        cBool enabled = ssao.ssaoSampleCount > 0;
        if (ImGui::Checkbox("SSAO 사용", &enabled))
        {
            if (enabled)
            {
                ssao.ssaoSampleCount = (savedSampleCount > 0) ? savedSampleCount : 16;
            }
            else
            {
                savedSampleCount = ssao.ssaoSampleCount;
                ssao.ssaoSampleCount = 0;
            }
        }
        if (ImGui::IsItemHovered())
        {
            // SSAO에서 오는 아티팩트인지 디퍼드 조명에서 오는지 가르는 실험용 토글
            ImGui::SetTooltip("끄면 SSAO 결과가 간접광에 반영되지 않는다.");
        }

        ImGui::Separator();

        ImGui::BeginDisabled(!enabled);

        if (ImGui::CollapsingHeader("파라미터", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderInt("샘플 수", &ssao.ssaoSampleCount, 1, 64);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("픽셀당 샘플 수. 많을수록 노이즈가 줄지만 비용이 비례해 늘어난다.");
            }

            ImGui::SliderFloat("반경", &ssao.ssaoRadius, 0.01f, 5.0f, "%.3f",
                               ImGuiSliderFlags_Logarithmic);
            if (ImGui::IsItemHovered())
            {
                // 씬 스케일에 맞아야 한다. 너무 크면 접촉 그림자 대신 넓은 번짐이 생긴다.
                ImGui::SetTooltip("월드 공간 탐색 반경. 씬 스케일에 맞아야 한다.");
            }

            ImGui::SliderFloat("바이어스", &ssao.ssaoBias, 0.0f, 0.2f, "%.4f");
            if (ImGui::IsItemHovered())
            {
                // 너무 작으면 표면에 줄무늬, 너무 크면 접촉 그림자가 사라진다.
                ImGui::SetTooltip("자기 가림으로 생기는 줄무늬를 막는 깊이 오프셋.");
            }

            ImGui::SliderFloat("강도", &ssao.ssaoPower, 0.1f, 8.0f, "%.2f");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("AO 에 적용하는 지수. 클수록 대비가 세진다.");
            }
        }

        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextDisabled("반경=%.3f  바이어스=%.4f  샘플=%d  강도=%.2f",
                            ssao.ssaoRadius, ssao.ssaoBias,
                            ssao.ssaoSampleCount, ssao.ssaoPower);

        ImGui::End();
    }

    void Application::renderHDRControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(480, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(470, 380), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("HDR 스카이박스 설정"))
        {
            ImGui::End();
            return;
        }

        // HDR Environment Controls
        if (ImGui::CollapsingHeader("HDR 환경", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("환경광 세기", &this->renderer.getSkyOptionsUBO().environmentIntensity, 0.0f,
                               10.0f, "%.2f");
        }

        // Environment Map Controls
        if (ImGui::CollapsingHeader("환경 맵", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("거칠기 단계", &this->renderer.getSkyOptionsUBO().roughnessLevel, 0.0f, 8.0f, "%.1f");

            bool useIrradiance = this->renderer.getSkyOptionsUBO().useIrradianceMap != 0;
            if (ImGui::Checkbox("Irradiance 맵 사용", &useIrradiance))
            {
                this->renderer.getSkyOptionsUBO().useIrradianceMap = useIrradiance ? 1 : 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("?"))
            {
                // Optional: Add click action here if needed
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("미리 필터링한 환경 맵(선명한 반사)과 "
                                  "irradiance 맵(부드러운 조명)을 번갈아 쓴다");
            }
        }

        // Debug Visualization
        if (ImGui::CollapsingHeader("디버그 표시"))
        {
            bool showMipLevels = this->renderer.getSkyOptionsUBO().showMipLevels != 0;
            if (ImGui::Checkbox("밉 단계 보기", &showMipLevels))
            {
                this->renderer.getSkyOptionsUBO().showMipLevels = showMipLevels ? 1 : 0;
            }

            bool showCubeFaces = this->renderer.getSkyOptionsUBO().showCubeFaces != 0;
            if (ImGui::Checkbox("큐브 면 보기", &showCubeFaces))
            {
                this->renderer.getSkyOptionsUBO().showCubeFaces = showCubeFaces ? 1 : 0;
            }
        }

        // Simplified Presets
        if (ImGui::CollapsingHeader("프리셋"))
        {
            if (ImGui::Button("기본값"))
            {
                this->renderer.getSkyOptionsUBO().environmentIntensity = 1.0f;
                this->renderer.getSkyOptionsUBO().roughnessLevel = 0.5f;
                this->renderer.getSkyOptionsUBO().useIrradianceMap = 0;
                this->renderer.getSkyOptionsUBO().showMipLevels = 0;
                this->renderer.getSkyOptionsUBO().showCubeFaces = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("노출 높게"))
            {
                this->renderer.getSkyOptionsUBO().environmentIntensity = 1.5f;
            }
            ImGui::SameLine();
            if (ImGui::Button("노출 낮게"))
            {
                this->renderer.getSkyOptionsUBO().environmentIntensity = 0.8f;
            }

            if (ImGui::Button("선명한 반사"))
            {
                this->renderer.getSkyOptionsUBO().roughnessLevel = 0.0f;
                this->renderer.getSkyOptionsUBO().useIrradianceMap = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("부드러운 조명"))
            {
                this->renderer.getSkyOptionsUBO().useIrradianceMap = 1;
            }
        }

        ImGui::End();
    }

    void Application::renderPostProcessingControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(960, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(520, 620), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("후처리 설정"))
        {
            ImGui::End();
            return;
        }

        // Tone Mapping Controls
        if (ImGui::CollapsingHeader("톤 매핑", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char *toneMappingNames[] = {"끄기", "Reinhard", "ACES",
                                              "Uncharted 2", "GT (Gran Turismo)", "Lottes",
                                              "Exponential", "Reinhard Extended", "Luminance",
                                              "Hable"};
            ImGui::Combo("톤 매핑 방식", &this->renderer.getPostProcessingOptionsUBO().toneMappingType,
                         toneMappingNames, IM_ARRAYSIZE(toneMappingNames));

            ImGui::SliderFloat("노출", &this->renderer.getPostProcessingOptionsUBO().exposure, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("감마", &this->renderer.getPostProcessingOptionsUBO().gamma, 1.0f / 2.2f, 2.2f, "%.2f");

            if (this->renderer.getPostProcessingOptionsUBO().toneMappingType == 7)
            { // Reinhard Extended
                ImGui::SliderFloat("최대 흰색", &this->renderer.getPostProcessingOptionsUBO().maxWhite, 1.0f, 20.0f,
                                   "%.1f");
            }
        }

        // Color Grading Controls
        if (ImGui::CollapsingHeader("색 보정", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("대비", &this->renderer.getPostProcessingOptionsUBO().contrast, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("밝기", &this->renderer.getPostProcessingOptionsUBO().brightness, -1.0f, 1.0f,
                               "%.2f");
            ImGui::SliderFloat("채도", &this->renderer.getPostProcessingOptionsUBO().saturation, 0.0f, 2.0f, "%.2f");
            ImGui::SliderFloat("생동감", &this->renderer.getPostProcessingOptionsUBO().vibrance, -1.0f, 1.0f, "%.2f");
        }

        // Effects Controls
        if (ImGui::CollapsingHeader("효과"))
        {
            ImGui::SliderFloat("비네트 세기", &this->renderer.getPostProcessingOptionsUBO().vignetteStrength, 0.0f,
                               1.0f, "%.2f");
            if (this->renderer.getPostProcessingOptionsUBO().vignetteStrength > 0.0f)
            {
                ImGui::SliderFloat("비네트 반경", &this->renderer.getPostProcessingOptionsUBO().vignetteRadius, 0.1f,
                                   1.5f, "%.2f");
            }

            ImGui::SliderFloat("필름 그레인", &this->renderer.getPostProcessingOptionsUBO().filmGrainStrength, 0.0f, 0.2f,
                               "%.3f");
            ImGui::SliderFloat("색수차", &this->renderer.getPostProcessingOptionsUBO().chromaticAberration,
                               0.0f, 5.0f, "%.1f");
        }

        // Debug Controls
        if (ImGui::CollapsingHeader("디버그 표시"))
        {
            const char *debugModeNames[] = {"끄기", "톤 매핑 비교", "색 채널",
                                            "분할 비교"};
            ImGui::Combo("디버그 모드", &this->renderer.getPostProcessingOptionsUBO().debugMode, debugModeNames,
                         IM_ARRAYSIZE(debugModeNames));

            if (this->renderer.getPostProcessingOptionsUBO().debugMode == 2)
            { // Color Channels
                const char *channelNames[] = {"전체", "R 만", "G 만",
                                              "B 만", "알파", "휘도"};
                ImGui::Combo("채널 보기", &this->renderer.getPostProcessingOptionsUBO().showOnlyChannel, channelNames,
                             IM_ARRAYSIZE(channelNames));
            }

            if (this->renderer.getPostProcessingOptionsUBO().debugMode == 3)
            { // Split Comparison
                ImGui::SliderFloat("분할 위치", &this->renderer.getPostProcessingOptionsUBO().debugSplit, 0.0f, 1.0f,
                                   "%.2f");
            }
        }

        // Presets
        if (ImGui::CollapsingHeader("프리셋"))
        {
            if (ImGui::Button("기본값"))
            {
                this->renderer.getPostProcessingOptionsUBO().toneMappingType = 2; // ACES
                this->renderer.getPostProcessingOptionsUBO().exposure = 1.0f;
                this->renderer.getPostProcessingOptionsUBO().gamma = 2.2f;
                this->renderer.getPostProcessingOptionsUBO().contrast = 1.0f;
                this->renderer.getPostProcessingOptionsUBO().brightness = 0.0f;
                this->renderer.getPostProcessingOptionsUBO().saturation = 1.0f;
                this->renderer.getPostProcessingOptionsUBO().vibrance = 0.0f;
                this->renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.0f;
                this->renderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.0f;
                this->renderer.getPostProcessingOptionsUBO().chromaticAberration = 0.0f;
                this->renderer.getPostProcessingOptionsUBO().debugMode = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("시네마틱"))
            {
                this->renderer.getPostProcessingOptionsUBO().toneMappingType = 3; // Uncharted 2
                this->renderer.getPostProcessingOptionsUBO().exposure = 1.2f;
                this->renderer.getPostProcessingOptionsUBO().contrast = 1.1f;
                this->renderer.getPostProcessingOptionsUBO().saturation = 0.9f;
                this->renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.3f;
                this->renderer.getPostProcessingOptionsUBO().vignetteRadius = 0.8f;
                this->renderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.02f;
            }

            if (ImGui::Button("대비 높게"))
            {
                this->renderer.getPostProcessingOptionsUBO().contrast = 1.5f;
                this->renderer.getPostProcessingOptionsUBO().brightness = 0.1f;
                this->renderer.getPostProcessingOptionsUBO().saturation = 1.3f;
                this->renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.2f;
            }
            ImGui::SameLine();
            if (ImGui::Button("대비 낮게"))
            {
                this->renderer.getPostProcessingOptionsUBO().contrast = 0.7f;
                this->renderer.getPostProcessingOptionsUBO().brightness = 0.05f;
                this->renderer.getPostProcessingOptionsUBO().saturation = 0.8f;
            }

            if (ImGui::Button("톤 매핑 비교"))
            {
                this->renderer.getPostProcessingOptionsUBO().debugMode = 1;
                this->renderer.getPostProcessingOptionsUBO().exposure = 2.0f;
            }
        }

        ImGui::End();
    }

    void Application::renderCameraControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(1970, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 430), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("카메라 설정"))
        {
            ImGui::End();
            return;
        }

        object::Camera2 &camera = *this->camera;
        cVec3 CameraPosition = this->camera->getPos();
        cFloat Yaw = this->camera->getYaw();
        cFloat Pithch = this->camera->getPitch();

        // Camera Information Display
        if (ImGui::CollapsingHeader("카메라 정보", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("위치 : (%.2f, %.2f, %.2f)", CameraPosition.x, CameraPosition.y, CameraPosition.z);
            ImGui::Text("Yaw / Pitch : (%.2f, %.2f)", Yaw, Pithch);

            // Camera Type Toggle
            /*bool isFirstPerson = camera_.type == hlab::Camera::CameraType::firstperson;
            if (ImGui::Checkbox("1인칭 모드", &isFirstPerson)) {
                camera_.type = isFirstPerson ? hlab::Camera::CameraType::firstperson
                    : hlab::Camera::CameraType::lookat;
            }*/
        }

        // Camera Position Controls
        if (ImGui::CollapsingHeader("위치 조절", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 tempPosition = camera.getPos();
            if (ImGui::SliderFloat3("위치", &tempPosition.x, -50.0f, 50.0f, "%.2f"))
            {
                camera.setPos(tempPosition);
            }

            // Quick position buttons
            if (ImGui::Button("위치 초기화"))
            {
                camera.setPos(glm::vec3(0.0f, 0.0f, -2.5f));
            }
            ImGui::SameLine();
            if (ImGui::Button("원점 보기"))
            {
                camera.setPos(glm::vec3(0.0f, 0.0f, 5.0f));
            }
        }

        // Camera Rotation Controls
        if (ImGui::CollapsingHeader("회전 조절", ImGuiTreeNodeFlags_DefaultOpen))
        {
            /*glm::vec3 tempRotation = camera.rotation;
            if (ImGui::SliderFloat3("회전 (도)", &tempRotation.x, -180.0f, 180.0f, "%.1f°")) {
                camera_.setRotation(tempRotation);
            }*/

            // Quick rotation buttons
            if (ImGui::Button("회전 초기화"))
            {
                camera.setPitch(cFloat(0.0f));
                camera.setYaw(cFloat(0.0f));
            }
            ImGui::SameLine();
        }

        // Controls Information
        if (ImGui::CollapsingHeader("조작법"))
        {
            ImGui::Text("키보드");
            ImGui::BulletText("WASD : 앞 / 뒤 / 좌 / 우 이동");
            ImGui::BulletText("Q / E : 위 / 아래 이동");
            ImGui::BulletText("F2 : 카메라 모드 전환");
            ImGui::BulletText("F3 : 카메라 정보를 콘솔에 출력");

            ImGui::Separator();
            ImGui::Text("마우스");
            ImGui::BulletText("왼쪽 드래그 : 시점 회전");
            ImGui::BulletText("오른쪽 드래그 : 확대 / 축소");
            ImGui::BulletText("가운데 드래그 : 평행 이동");
            ImGui::BulletText("휠 : 확대 / 축소");
        }

        ImGui::End();
    }

}
