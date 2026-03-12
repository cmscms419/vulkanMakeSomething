#include "Application3.h"

#include "helper.h"
#include "vkconfig.h"
#include "log.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Application3.h"

using namespace vkengine::Log;

namespace vkengine
{
    Application3::Application3(cString root_path) : Application3(ApplicationConfig::createDefault(), root_path)
    {
    }

    Application3::Application3(const ApplicationConfig &config, cString root_path)
        : VulkanEngineWin2(true),
          RootPath{root_path.c_str()},
          shaderManager{*this->cxt,
                        RootPath + SHADER_PATH,
                        {
                            {"shadowMap", {"vertshadowMap.spv", "fragshadowMap.spv"}},
                            {"pbrForward", {"vertpbrForward.spv", "fragpbrForward.spv"}},
                            {"gui", {"vertimgui.spv", "fragimgui.spv"}},
                            {"sky", {"vertskybox2.spv", "fragskybox2.spv"}},
                            {"post", {"vertpost.spv", "fragpost.spv"}},
                            {"ssao", {"compssao.spv"}},
                        }},
          guiRenderer{*this->cxt, shaderManager, swapChain->getSwapChainImageFormat(), RootPath + this->AssetsPath},
          Renderer(*this->cxt, shaderManager, this->kMaxFramesInFlight, RootPath + this->AssetsPath, RootPath + this->ShaderPath)
    {
        initializeVulkanResources();
        setupCallbacks();
        initializeWithConfig(config);
        initializeRenderGraph();
    }

    Application3::~Application3()
    {
    }

    void Application3::initializeWithConfig(const ApplicationConfig &config)
    {
        this->loadModels(config.models);

        this->Renderer.prepareForModels(
            this->models,
            this->swapChain->getSwapChainImageFormat(),
            this->cxt->getDepthStencil()->depthFormat,
            this->msaaSamples,
            this->extent.width,
            this->extent.height);
    }

    void Application3::loadModels(const std::vector<ModelConfig> &modelConfigs)
    {
        for (const auto &modelConfig : modelConfigs)
        {
            this->models.emplace_back(*cxt);
            auto &model = this->models.back();

            cString fullPath = this->RootPath + this->AssetsPath + modelConfig.filePath;
            model.loadFromModelFile(fullPath, modelConfig.isBistroObj);
            model.Name() = modelConfig.displayName;
            model.ModelResource().modelMatrix = modelConfig.transform;

            PRINT_TO_LOGGER("Loaded model '%s'\n", modelConfig.displayName.c_str());
            PRINT_TO_LOGGER("   - Meshes: %zu\n", model.Meshes().size());
            PRINT_TO_LOGGER("   - Materials: %zu\n", model.Materials().size());
            PRINT_TO_LOGGER("   - Textures: %zu\n", model.Textures().size());

            // Setup animation if model supports it
            if (model.hasAnimations() && modelConfig.autoPlayAnimation)
            {
                PRINT_TO_LOGGER("   Animations: %d found\n", model.getAnimationCount());

                if (model.getAnimationCount() > 0)
                {
                    uint32_t animIndex = std::min(modelConfig.initialAnimationIndex,
                                                  model.getAnimationCount() - 1);
                    model.setAnimationIndex(animIndex);
                    model.setAnimationLooping(modelConfig.loopAnimation);
                    model.setAnimationSpeed(modelConfig.animationSpeed);
                    model.playAnimation();

                    PRINT_TO_LOGGER("   - Started animation: '%s' (%.2f sec)\n",
                                    model.getAnimation()->getCurrentAnimationName().c_str(),
                                    model.getAnimation()->getDuration());
                }
            }
            else if (!model.hasAnimations())
            {
                PRINT_TO_LOGGER("   Animations: None\n");
            }

            PRINT_TO_LOGGER("\n"); // 빈 줄
        }
    }

    void Application3::setupCallbacks()
    {
        // engine에서 제공하는 카메라 컨트롤 윈도우 사용
        this->window->setCamera(this->camera);
    }
    
    void Application3::initializeVulkanResources()
    {
        this->msaaSamples = helper::device::getMaxUsableSampleCount(this->cxt->getDevice()->physicalDevice);
        this->commandBuffers = this->cxt->createGrapicsCommandBufferHanders(this->kMaxFramesInFlight);
        this->camera = std::make_shared<vkengine::object::Camera2>();

        cFloat fov = camera->getFov();
        cFloat nearP = camera->getNearP();
        cFloat farP = camera->getFarP();
        cFloat aspectRatio = static_cast<cFloat>(this->extent.width) / static_cast<cFloat>(this->extent.height);
        camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);

        // engine에서 이미 fence와 semaphore를 생성하므로 생략
    }

    void Application3::initializeRenderGraph()
    {
        this->Renderer.buildRenderGraph(*this->swapChain);
    }

    void Application3::run()
    {
        _VK_CHECK_RESULT_(vkWaitForFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame]));

        Renderer.update(*camera.get(), currentFrame, (float)glfwGetTime() * 0.5f);
        // Renderer.updateBoneData(models, currentFrame);

        cMat4 viewProjection = this->camera->getProjectionMatrix() * this->camera->getViewMatrix();
        Renderer.updateViewFrustum(viewProjection);

        for (auto &model : models)
        {
            const glm::mat4 &modelMatrix = model.ModelResource().modelMatrix;
            for (auto &mesh : model.Meshes())
            {
                mesh.updateWorldBounds(modelMatrix);
            }
        }

        // Perform frustum culling on all models
        Renderer.performFrustumCulling(models);
        guiRenderer.update();

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
            EXIT_TO_LOGGER("Could not acquire the next swap chain image!\n");
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

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            this->framebufferResized = false;
            recreateSwapchain();
        } else if (presentResult != VK_SUCCESS) {
            EXIT_TO_LOGGER("Failed to present swapchain image!\n");
        }
    }

    void Application3::update()
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

            updateGui();

            this->camera->update();
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

    void Application3::updateGui()
    {
        static float scale = 1.4f;

        ImGuiIO &io = ImGui::GetIO();

        io.DisplaySize = ImVec2(float(extent.width), float(extent.height));
        // io.DeltaTime = frameTimer;
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
        ImGui::Begin("벌컨 실시간 렌더링 예제", nullptr, ImGuiWindowFlags_None);

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

        // Rendering Options Controls
        ImGui::Separator();
        ImGui::Text("Rendering Options");

        bool textureOn = this->Renderer.getOptionsUniform().textureOn != 0;
        bool shadowOn = this->Renderer.getOptionsUniform().shadowOn != 0;
        bool discardOn = this->Renderer.getOptionsUniform().discardOn != 0;

        // NEW: Frustum Culling Controls
        ImGui::Separator();
        ImGui::Text("View Frustum Culling");

        bool frustumCullingEnabled = this->Renderer.isFrustumCullingEnabled();
        if (ImGui::Checkbox("Enable Frustum Culling", &frustumCullingEnabled))
        {
            this->Renderer.setFrustumCullingEnabled(frustumCullingEnabled);
        }

        // Display culling statistics
        const auto &stats = this->Renderer.getCullingStats();
        ImGui::Text("Total Meshes: %u", stats.totalMeshes);
        ImGui::Text("Rendered: %u", stats.renderedMeshes);
        ImGui::Text("Culled: %u", stats.culledMeshes);

        if (stats.totalMeshes > 0)
        {
            float cullPercent = (float)stats.culledMeshes / stats.totalMeshes * 100.0f;
            ImGui::Text("Culled: %.1f%%", cullPercent);
        }

        if (ImGui::Checkbox("Textures", &textureOn))
        {
            this->Renderer.getOptionsUniform().textureOn = textureOn ? 1 : 0;
        }
        if (ImGui::Checkbox("Shadows", &shadowOn))
        {
            this->Renderer.getOptionsUniform().shadowOn = shadowOn ? 1 : 0;
        }
        if (ImGui::Checkbox("Alpha Discard", &discardOn))
        {
            this->Renderer.getOptionsUniform().discardOn = discardOn ? 1 : 0;
        }

        ImGui::Separator();

        for (uint32_t i = 0; i < models.size(); i++)
        {
            auto &m = models[i];
            char labelBuffer[128];

            // 모델 이름
            cString modelName = m.Name().empty() ? "Unnamed" : m.Name();
            snprintf(labelBuffer, sizeof(labelBuffer), "Model %u - %s", i, modelName.c_str());

            // clean
            if (ImGui::TreeNode(labelBuffer))
            {
                // Visible 체크박스
                snprintf(labelBuffer, sizeof(labelBuffer), "Visible##%u", i);
                ImGui::Checkbox(labelBuffer, &m.Visible());

                // Coefficients 슬라이더들
                float *coeffs = m.ModelResource().coeffs;

                snprintf(labelBuffer, sizeof(labelBuffer), "Specular##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[0], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Diffuse##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[1], 0.0f, 10.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Emissive##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[2], 0.0f, 10.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Shadow Offset##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[3], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Metallic##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[4], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Roughness##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[5], 0.0f, 1.0f);

                ImGui::TreePop();
            }

            // Extract and edit position
            char labelBuffer2[128];
            glm::vec3 position = glm::vec3(m.ModelResource().modelMatrix[3]);

            snprintf(labelBuffer2, sizeof(labelBuffer2), "Position##%u", i);

            if (ImGui::SliderFloat3(labelBuffer2, &position.x, -10.0f, 10.0f))
            {
                m.ModelResource().modelMatrix[3] = glm::vec4(position, 1.0f);
            }

            // Decompose matrix into components
            glm::vec3 scale, translation, skew;
            glm::vec4 perspective;
            glm::quat rotation;

            if (glm::decompose(m.ModelResource().modelMatrix, scale, rotation, translation, skew, perspective))
            {
                // Convert quaternion to euler angles for easier editing
                glm::vec3 eulerAngles = glm::eulerAngles(rotation);
                float yRotationDegrees = glm::degrees(eulerAngles.y);
                char labelBuffer3[128];

                snprintf(labelBuffer3, sizeof(labelBuffer3), "Y Rotation##%u", i);
                if (ImGui::SliderFloat(labelBuffer3, &yRotationDegrees, -90.0f, 90.0f, "%.1f°"))
                {
                    // Reconstruct matrix from components
                    eulerAngles.y = glm::radians(yRotationDegrees);
                    rotation = glm::quat(eulerAngles);

                    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
                    glm::mat4 R = glm::mat4_cast(rotation);
                    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

                    m.ModelResource().modelMatrix = T * R * S;
                }
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();

        // Camera Control Window
        renderCameraControlWindow();

        renderHDRControlWindow();

        renderPostProcessingControlWindow();

        ImGui::Render();
    }

    void Application3::renderHDRControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("HDR Skybox Controls"))
        {
            ImGui::End();
            return;
        }

        // HDR Environment Controls
        if (ImGui::CollapsingHeader("HDR Environment", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Environment Intensity", &this->Renderer.getSkyOptionsUBO().environmentIntensity, 0.0f,
                               10.0f, "%.2f");
        }

        // Environment Map Controls
        if (ImGui::CollapsingHeader("Environment Map", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Roughness Level", &this->Renderer.getSkyOptionsUBO().roughnessLevel, 0.0f, 8.0f, "%.1f");

            bool useIrradiance = this->Renderer.getSkyOptionsUBO().useIrradianceMap != 0;
            if (ImGui::Checkbox("Use Irradiance Map", &useIrradiance))
            {
                this->Renderer.getSkyOptionsUBO().useIrradianceMap = useIrradiance ? 1 : 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("?"))
            {
                // Optional: Add click action here if needed
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Toggle between prefiltered environment map (sharp reflections) and "
                                  "irradiance map (diffuse lighting)");
            }
        }

        // Debug Visualization
        if (ImGui::CollapsingHeader("Debug Visualization"))
        {
            bool showMipLevels = this->Renderer.getSkyOptionsUBO().showMipLevels != 0;
            if (ImGui::Checkbox("Show Mip Levels", &showMipLevels))
            {
                this->Renderer.getSkyOptionsUBO().showMipLevels = showMipLevels ? 1 : 0;
            }

            bool showCubeFaces = this->Renderer.getSkyOptionsUBO().showCubeFaces != 0;
            if (ImGui::Checkbox("Show Cube Faces", &showCubeFaces))
            {
                this->Renderer.getSkyOptionsUBO().showCubeFaces = showCubeFaces ? 1 : 0;
            }
        }

        // Simplified Presets
        if (ImGui::CollapsingHeader("Presets"))
        {
            if (ImGui::Button("Default"))
            {
                this->Renderer.getSkyOptionsUBO().environmentIntensity = 1.0f;
                this->Renderer.getSkyOptionsUBO().roughnessLevel = 0.5f;
                this->Renderer.getSkyOptionsUBO().useIrradianceMap = 0;
                this->Renderer.getSkyOptionsUBO().showMipLevels = 0;
                this->Renderer.getSkyOptionsUBO().showCubeFaces = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("High Exposure"))
            {
                this->Renderer.getSkyOptionsUBO().environmentIntensity = 1.5f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Low Exposure"))
            {
                this->Renderer.getSkyOptionsUBO().environmentIntensity = 0.8f;
            }

            if (ImGui::Button("Sharp Reflections"))
            {
                this->Renderer.getSkyOptionsUBO().roughnessLevel = 0.0f;
                this->Renderer.getSkyOptionsUBO().useIrradianceMap = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Diffuse Lighting"))
            {
                this->Renderer.getSkyOptionsUBO().useIrradianceMap = 1;
            }
        }

        ImGui::End();
    }

    void Application3::renderPostProcessingControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(680, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Post-Processing Controls"))
        {
            ImGui::End();
            return;
        }

        // Tone Mapping Controls
        if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char *toneMappingNames[] = {"None", "Reinhard", "ACES",
                                              "Uncharted 2", "GT (Gran Turismo)", "Lottes",
                                              "Exponential", "Reinhard Extended", "Luminance",
                                              "Hable"};
            ImGui::Combo("Tone Mapping Type", &this->Renderer.getPostProcessingOptionsUBO().toneMappingType,
                         toneMappingNames, IM_ARRAYSIZE(toneMappingNames));

            ImGui::SliderFloat("Exposure", &this->Renderer.getPostProcessingOptionsUBO().exposure, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("Gamma", &this->Renderer.getPostProcessingOptionsUBO().gamma, 1.0f / 2.2f, 2.2f, "%.2f");

            if (this->Renderer.getPostProcessingOptionsUBO().toneMappingType == 7)
            { // Reinhard Extended
                ImGui::SliderFloat("Max White", &this->Renderer.getPostProcessingOptionsUBO().maxWhite, 1.0f, 20.0f,
                                   "%.1f");
            }
        }

        // Color Grading Controls
        if (ImGui::CollapsingHeader("Color Grading", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Contrast", &this->Renderer.getPostProcessingOptionsUBO().contrast, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("Brightness", &this->Renderer.getPostProcessingOptionsUBO().brightness, -1.0f, 1.0f,
                               "%.2f");
            ImGui::SliderFloat("Saturation", &this->Renderer.getPostProcessingOptionsUBO().saturation, 0.0f, 2.0f, "%.2f");
            ImGui::SliderFloat("Vibrance", &this->Renderer.getPostProcessingOptionsUBO().vibrance, -1.0f, 1.0f, "%.2f");
        }

        // Effects Controls
        if (ImGui::CollapsingHeader("Effects"))
        {
            ImGui::SliderFloat("Vignette Strength", &this->Renderer.getPostProcessingOptionsUBO().vignetteStrength, 0.0f,
                               1.0f, "%.2f");
            if (this->Renderer.getPostProcessingOptionsUBO().vignetteStrength > 0.0f)
            {
                ImGui::SliderFloat("Vignette Radius", &this->Renderer.getPostProcessingOptionsUBO().vignetteRadius, 0.1f,
                                   1.5f, "%.2f");
            }

            ImGui::SliderFloat("Film Grain", &this->Renderer.getPostProcessingOptionsUBO().filmGrainStrength, 0.0f, 0.2f,
                               "%.3f");
            ImGui::SliderFloat("Chromatic Aberration", &this->Renderer.getPostProcessingOptionsUBO().chromaticAberration,
                               0.0f, 5.0f, "%.1f");
        }

        // Debug Controls
        if (ImGui::CollapsingHeader("Debug Visualization"))
        {
            const char *debugModeNames[] = {"Off", "Tone Mapping Comparison", "Color Channels",
                                            "Split Comparison"};
            ImGui::Combo("Debug Mode", &this->Renderer.getPostProcessingOptionsUBO().debugMode, debugModeNames,
                         IM_ARRAYSIZE(debugModeNames));

            if (this->Renderer.getPostProcessingOptionsUBO().debugMode == 2)
            { // Color Channels
                const char *channelNames[] = {"All", "Red Only", "Green Only",
                                              "Blue Only", "Alpha", "Luminance"};
                ImGui::Combo("Show Channel", &this->Renderer.getPostProcessingOptionsUBO().showOnlyChannel, channelNames,
                             IM_ARRAYSIZE(channelNames));
            }

            if (this->Renderer.getPostProcessingOptionsUBO().debugMode == 3)
            { // Split Comparison
                ImGui::SliderFloat("Split Position", &this->Renderer.getPostProcessingOptionsUBO().debugSplit, 0.0f, 1.0f,
                                   "%.2f");
            }
        }

        // Presets
        if (ImGui::CollapsingHeader("Presets"))
        {
            if (ImGui::Button("Default"))
            {
                this->Renderer.getPostProcessingOptionsUBO().toneMappingType = 2; // ACES
                this->Renderer.getPostProcessingOptionsUBO().exposure = 1.0f;
                this->Renderer.getPostProcessingOptionsUBO().gamma = 2.2f;
                this->Renderer.getPostProcessingOptionsUBO().contrast = 1.0f;
                this->Renderer.getPostProcessingOptionsUBO().brightness = 0.0f;
                this->Renderer.getPostProcessingOptionsUBO().saturation = 1.0f;
                this->Renderer.getPostProcessingOptionsUBO().vibrance = 0.0f;
                this->Renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.0f;
                this->Renderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.0f;
                this->Renderer.getPostProcessingOptionsUBO().chromaticAberration = 0.0f;
                this->Renderer.getPostProcessingOptionsUBO().debugMode = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cinematic"))
            {
                this->Renderer.getPostProcessingOptionsUBO().toneMappingType = 3; // Uncharted 2
                this->Renderer.getPostProcessingOptionsUBO().exposure = 1.2f;
                this->Renderer.getPostProcessingOptionsUBO().contrast = 1.1f;
                this->Renderer.getPostProcessingOptionsUBO().saturation = 0.9f;
                this->Renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.3f;
                this->Renderer.getPostProcessingOptionsUBO().vignetteRadius = 0.8f;
                this->Renderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.02f;
            }

            if (ImGui::Button("High Contrast"))
            {
                this->Renderer.getPostProcessingOptionsUBO().contrast = 1.5f;
                this->Renderer.getPostProcessingOptionsUBO().brightness = 0.1f;
                this->Renderer.getPostProcessingOptionsUBO().saturation = 1.3f;
                this->Renderer.getPostProcessingOptionsUBO().vignetteStrength = 0.2f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Low Contrast"))
            {
                this->Renderer.getPostProcessingOptionsUBO().contrast = 0.7f;
                this->Renderer.getPostProcessingOptionsUBO().brightness = 0.05f;
                this->Renderer.getPostProcessingOptionsUBO().saturation = 0.8f;
            }

            if (ImGui::Button("Show Tone Mapping"))
            {
                this->Renderer.getPostProcessingOptionsUBO().debugMode = 1;
                this->Renderer.getPostProcessingOptionsUBO().exposure = 2.0f;
            }
        }

        ImGui::End();
    }

    void Application3::renderCameraControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(10, 350), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Camera Controls"))
        {
            ImGui::End();
            return;
        }

        object::Camera2 &camera = *this->camera;
        cVec3 CameraPosition = this->camera->getPos();
        cFloat Yaw = this->camera->getYaw();
        cFloat Pithch = this->camera->getPitch();

        // Camera Information Display
        if (ImGui::CollapsingHeader("Camera Information", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", CameraPosition.x, CameraPosition.y, CameraPosition.z);
            ImGui::Text("Camera Yaw Pitch: (%.2f, %.2f)", Yaw, Pithch);

            // Camera Type Toggle
            /*bool isFirstPerson = camera_.type == hlab::Camera::CameraType::firstperson;
            if (ImGui::Checkbox("First Person Mode", &isFirstPerson)) {
                camera_.type = isFirstPerson ? hlab::Camera::CameraType::firstperson
                    : hlab::Camera::CameraType::lookat;
            }*/
        }

        // Camera Position Controls
        if (ImGui::CollapsingHeader("Position Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 tempPosition = camera.getPos();
            if (ImGui::SliderFloat3("Position", &tempPosition.x, -50.0f, 50.0f, "%.2f"))
            {
                camera.setPos(tempPosition);
            }

            // Quick position buttons
            if (ImGui::Button("Reset Position"))
            {
                camera.setPos(glm::vec3(0.0f, 0.0f, -2.5f));
            }
            ImGui::SameLine();
            if (ImGui::Button("View Origin"))
            {
                camera.setPos(glm::vec3(0.0f, 0.0f, 5.0f));
            }
        }

        // Camera Rotation Controls
        if (ImGui::CollapsingHeader("Rotation Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            /*glm::vec3 tempRotation = camera.rotation;
            if (ImGui::SliderFloat3("Rotation (degrees)", &tempRotation.x, -180.0f, 180.0f, "%.1f°")) {
                camera_.setRotation(tempRotation);
            }*/

            // Quick rotation buttons
            if (ImGui::Button("Reset Rotation"))
            {
                camera.setPitch(cFloat(0.0f));
                camera.setYaw(cFloat(0.0f));
            }
            ImGui::SameLine();
        }

        // Controls Information
        if (ImGui::CollapsingHeader("Controls Help"))
        {
            ImGui::Text("Keyboard Controls:");
            ImGui::BulletText("WASD: Move forward/back/left/right");
            ImGui::BulletText("Q/E: Move up/down");
            ImGui::BulletText("F2: Toggle camera mode");
            ImGui::BulletText("F3: Print camera info to console");

            ImGui::Separator();
            ImGui::Text("Mouse Controls:");
            ImGui::BulletText("Left Click + Drag: Look around");
            ImGui::BulletText("Right Click + Drag: Zoom in/out");
            ImGui::BulletText("Middle Click + Drag: Pan");
            ImGui::BulletText("Scroll Wheel: Zoom");
        }

        ImGui::End();
    }

}
