#include "Application2.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace vkengine::Log;

namespace vkengine {

    Application2::Application2(cString root_path) :
        Application2(ApplicationConfig::createDefault(), root_path)
    {
    }

    Application2::Application2(const ApplicationConfig& config, cString root_path)
        :
        VulkanEngineWin2(true),
        RootPath{ root_path.c_str()},
        shaderManager{ *this->cxt,
                       RootPath + SHADER_PATH,
                       {
                           {"shadowMap", {"vertshadowMap.spv", "fragshadowMap.spv"}},
                           {"pbrForward", {"vertpbrForward.spv", "fragpbrForward.spv"}},
                           {"gui", {"vertimgui.spv", "fragimgui.spv"}},
                           {"sky", {"vertskybox2.spv", "fragskybox2.spv"}},
                           {"post", {"vertpost.spv", "fragpost.spv"}},
                           {"ssao", {"compssao.spv"}},
                       }
        },
        guiRenderer{ *this->cxt, shaderManager, swapChain->getSwapChainImageFormat(), RootPath + this->AssetsPath },
        forwardRenderer(*this->cxt, shaderManager, this->kMaxFramesInFlight, RootPath + this->AssetsPath, RootPath + this->ShaderPath)
    {
        initializeVulkanResources();
        setupCallbacks();
        initializeWithConfig(config);
    }

    Application2::Application2(const cString& configFile, cString root_path) :
        Application2(ApplicationConfig::createDefault(), root_path)
    {
        PRINT_TO_LOGGER("Config file loading not implemented yet, using default configuration\n");
    }

    Application2::~Application2()
    {
    }

    void Application2::update()
    {
        // 파이프라인은 어떤 레이아웃으로 리소스가 들어와야 하는지는 알고 있지만
        // 구체적으로 어떤 리소스가 들어올지를 직접 결정하지는 않는다.
        // 렌더러가 파이프라인을 사용할 때 어떤 리소스를 넣을지 결정한다.

        // NEW: Animation timing variables
        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.016f; // Default to ~60 FPS

        while (!window->shouldClose()) {
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

            // Update camera
            this->camera->update();
            this->forwardRenderer.getSceneDataUBO().projection = this->camera->getProjectionMatrix();
            this->forwardRenderer.getSceneDataUBO().view = this->camera->getViewMatrix();
            this->forwardRenderer.getSceneDataUBO().cameraPos = camera->getPos();

            for (auto& model : models) {
                if (model.hasAnimations()) {
                    model.updateAnimation(deltaTime);
                }
            }
            
            //1.    Tight Fitting: 씬의 모든 오브젝트를 정확히 감싸는 최소 크기의 광원 절두체 생성
            //2.    Shadow Map 해상도 최적화 : 불필요한 공간을 제거하여 섀도우 맵의 해상도를 효율적으로 사용
            //3.    Directional Light : 태양과 같은 평행광을 시뮬레이션하기 위해 직교 투영 사용
            // Update for shadow mapping
            if (models.size() > 0) {

                cMat4 lightView = glm::lookAt(cVec3(0.0f), -forwardRenderer.getSceneDataUBO().directionalLightDir, cVec3(0.0f, 0.0f, 1.0f));

                // Transform the first model's bounding box to find the initial light bounding box
                // 초기 빛 경계 상자를 찾기 위해 첫 번째 모델의 경계 상자를 변환합니다.
                cVec3 firstMin =
                    cVec3(models[0].ModelMatrix() * cVec4(models[0].BoundingBoxMin(), 1.0f));
                cVec3 firstMax =
                    cVec3(models[0].ModelMatrix() * cVec4(models[0].BoundingBoxMax(), 1.0f));

                // Ensure min is actually smaller than max for each component
                // 각 구성 요소에 대해 최소값이 실제로 최대값보다 작은지 확인합니다.
                cVec3 min_ = glm::min(firstMin, firstMax);
                cVec3 max_ = glm::max(firstMin, firstMax);

                // Iterate through all models to find the combined bounding box
                // 모든 모델을 반복하여 결합된 경계 상자를 찾습니다.
                for (uint32_t i = 1; i < models.size(); i++) {
                    // Transform this model's bounding box to world space
                    // 이 모델의 경계 상자를 월드 공간으로 변환합니다.
                    cVec3 modelMin = cVec3(models[i].ModelMatrix() * cVec4(models[i].BoundingBoxMin(), 1.0f));
                    cVec3 modelMax = cVec3(models[i].ModelMatrix() * cVec4(models[i].BoundingBoxMax(), 1.0f));

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
                    cVec3(min_.x, min_.y, min_.z), cVec3(min_.x, max_.y, min_.z),
                    cVec3(min_.x, min_.y, max_.z), cVec3(min_.x, max_.y, max_.z),
                    cVec3(max_.x, min_.y, min_.z), cVec3(max_.x, max_.y, min_.z),
                    cVec3(max_.x, min_.y, max_.z), cVec3(max_.x, max_.y, max_.z),
                };
                cVec3 vmin(std::numeric_limits<float>::max());
                cVec3 vmax(std::numeric_limits<float>::lowest());
                for (size_t i = 0; i != 8; i++) {
                    auto temp = cVec3(lightView * cVec4(corners[i], 1.0f));
                    vmin = glm::min(vmin, temp);
                    vmax = glm::max(vmax, temp);
                }
                min_ = vmin;
                max_ = vmax;
                cMat4 lightProjection = glm::orthoLH_ZO(min_.x, max_.x, min_.y, max_.y, max_.z, min_.z); // 마지막 Max, Min 순서 주의
                forwardRenderer.getSceneDataUBO().lightSpaceMatrix = lightProjection * lightView;

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

    void Application2::run()
    {
        // Wait using currentFrame index (CPU-side fence)
        _VK_CHECK_RESULT_(vkWaitForFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(this->cxt->getDevice()->logicaldevice, 1, &this->inFlightFences[currentFrame]));

        forwardRenderer.update(*camera.get(), currentFrame, (float)glfwGetTime() * 0.5f);
        forwardRenderer.updateBoneData(models, currentFrame);

        // NEW: Update view frustum and perform culling
        cMat4 viewProjection = this->camera->getProjectionMatrix() * this->camera->getViewMatrix();
        forwardRenderer.updateViewFrustum(viewProjection);

        // Update world bounds for all meshes before performing culling
        for (auto& model : models) {
            const glm::mat4& modelMatrix = model.ModelMatrix();
            for (auto& mesh : model.Meshes()) {
                mesh.updateWorldBounds(modelMatrix);
            }
        }

        // Perform frustum culling on all models
        forwardRenderer.performFrustumCulling(models);
        guiRenderer.update();

        // Acquire using currentSemaphore index (GPU-side semaphore)
        uint32_t imageIndex{ 0 };
        VkResult result = this->swapChain->acquireNextImage(presentSemaphores[currentSemaphore], imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return; // Ignore resize in this example
        }
        else if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR)) {
            EXIT_TO_LOGGER("Could not acquire the next swap chain image!\n");
        }

        // Use currentFrame index (CPU-side command buffer)
        VKCommandBufferHander& cmd = this->commandBuffers[currentFrame];

        // Begin command buffer
        vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
        VkCommandBufferBeginInfo cmdBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        
        _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));

        // Make Shadow map
        {
            forwardRenderer.makeShadowMap(cmd.getCommandBuffer(), currentFrame, models);
        }

        {
            // Transition swapchain image from undefined to color attachment layout
            this->swapChain->transitionTo(
                    cmd.getCommandBuffer(),
                    imageIndex,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

            VkViewport viewport{ 0.0f, 0.0f, (float)this->extent.width, (float)this->extent.height,
                                0.0f, 1.0f };
            VkRect2D scissor{ 0, 0, this->extent.width, this->extent.height };

            // Draw models
            this->forwardRenderer.draw(cmd.getCommandBuffer(), currentFrame, this->swapChain->getSwapChainImageView(imageIndex), models,viewport, scissor);

            // Draw GUI (overwrite to swapchain image)
            this->guiRenderer.draw(cmd.getCommandBuffer(), this->swapChain->getSwapChainImageView(imageIndex), viewport);

            this->swapChain->transitionTo(
                cmd.getCommandBuffer(),
                imageIndex,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
        }
        _VK_CHECK_RESULT_(vkEndCommandBuffer(cmd.getCommandBuffer())); // End command buffer

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // 비교: 마지막으로 실행되는 셰이더가 Compute라면 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pCommandBuffers = &cmd.getCommandBuffer();
        submitInfo.commandBufferCount = 1;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.pWaitSemaphores = &this->presentSemaphores[currentSemaphore];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &this->renderSemaphores[currentSemaphore];
        submitInfo.signalSemaphoreCount = 1;
        _VK_CHECK_RESULT_(vkQueueSubmit(cmd.getQueue(), 1, &submitInfo, this->inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &this->renderSemaphores[currentSemaphore];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &this->swapChain->getSwapChain();
        presentInfo.pImageIndices = &imageIndex;
        _VK_CHECK_RESULT_(vkQueuePresentKHR(this->cxt->grapicsQueue(), &presentInfo));
    }

    void Application2::updateGui()
    {
        static float scale = 1.4f;

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(float(extent.width), float(extent.height));
        // io.DeltaTime = frameTimer;
        platform::MouseState& mousetState = this->window->getmouseState();  // Update mouse state

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
        this->forwardRenderer.getSceneDataUBO().directionalLightColor = lightIntensity * lightColor;

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
        this->forwardRenderer.getSceneDataUBO().directionalLightDir = lightDir;

        // Display current light direction for debugging
        ImGui::Text("Light Dir: (%.2f, %.2f, %.2f)", 
            this->forwardRenderer.getSceneDataUBO().directionalLightDir.x,
            this->forwardRenderer.getSceneDataUBO().directionalLightDir.y,
            this->forwardRenderer.getSceneDataUBO().directionalLightDir.z);

        // Rendering Options Controls
        ImGui::Separator();
        ImGui::Text("Rendering Options");

        bool textureOn = this->forwardRenderer.getOptionsUniform().textureOn != 0;
        bool shadowOn = this->forwardRenderer.getOptionsUniform().shadowOn != 0;
        bool discardOn = this->forwardRenderer.getOptionsUniform().discardOn != 0;

        // NEW: Frustum Culling Controls
        ImGui::Separator();
        ImGui::Text("View Frustum Culling");

        bool frustumCullingEnabled = this->forwardRenderer.isFrustumCullingEnabled();
        if (ImGui::Checkbox("Enable Frustum Culling", &frustumCullingEnabled)) {
            this->forwardRenderer.setFrustumCullingEnabled(frustumCullingEnabled);
        }

        // Display culling statistics
        const auto& stats = this->forwardRenderer.getCullingStats();
        ImGui::Text("Total Meshes: %u", stats.totalMeshes);
        ImGui::Text("Rendered: %u", stats.renderedMeshes);
        ImGui::Text("Culled: %u", stats.culledMeshes);

        if (stats.totalMeshes > 0) {
            float cullPercent = (float)stats.culledMeshes / stats.totalMeshes * 100.0f;
            ImGui::Text("Culled: %.1f%%", cullPercent);
        }

        if (ImGui::Checkbox("Textures", &textureOn)) {
            this->forwardRenderer.getOptionsUniform().textureOn = textureOn ? 1 : 0;
        }
        if (ImGui::Checkbox("Shadows", &shadowOn)) {
            this->forwardRenderer.getOptionsUniform().shadowOn = shadowOn ? 1 : 0;
        }
        if (ImGui::Checkbox("Alpha Discard", &discardOn)) {
            this->forwardRenderer.getOptionsUniform().discardOn = discardOn ? 1 : 0;
        }
        // if (ImGui::Checkbox("Animation", &animationOn)) {
        //     renderer_.optionsUBO().animationOn = animationOn ? 1 : 0;
        // }

        ImGui::Separator();

        for (uint32_t i = 0; i < models.size(); i++) {
            auto& m = models[i];
            char labelBuffer[128];

            // 모델 이름
            cString modelName = m.Name().empty() ? "Unnamed" : m.Name();
            snprintf(labelBuffer, sizeof(labelBuffer), "Model %u - %s", i, modelName.c_str());

            // clean
            if (ImGui::TreeNode(labelBuffer)) {
                // Visible 체크박스
                snprintf(labelBuffer, sizeof(labelBuffer), "Visible##%u", i);
                ImGui::Checkbox(labelBuffer, &m.Visible());

                // Coefficients 슬라이더들
                float* coeffs = m.Coeffs();

                snprintf(labelBuffer, sizeof(labelBuffer), "Specular##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[0], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Diffuse##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[1], 0.0f, 10.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Emissive##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[2], 0.0f, 10.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Shadow Offset##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[3], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Roughness##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[4], 0.0f, 1.0f);

                snprintf(labelBuffer, sizeof(labelBuffer), "Metallic##%u", i);
                ImGui::SliderFloat(labelBuffer, &coeffs[5], 0.0f, 1.0f);

                ImGui::TreePop();
            }

            // Extract and edit position
            char labelBuffer2[128];
            glm::vec3 position = glm::vec3(m.ModelMatrix()[3]);
            
            snprintf(labelBuffer2, sizeof(labelBuffer2), "Position##%u", i);
            
            if (ImGui::SliderFloat3(labelBuffer2, &position.x, -10.0f, 10.0f))
            {
                m.ModelMatrix()[3] = glm::vec4(position, 1.0f);
            }

            // Decompose matrix into components
            glm::vec3 scale, translation, skew;
            glm::vec4 perspective;
            glm::quat rotation;

            if (glm::decompose(m.ModelMatrix(), scale, rotation, translation, skew, perspective)) {
                // Convert quaternion to euler angles for easier editing
                glm::vec3 eulerAngles = glm::eulerAngles(rotation);
                float yRotationDegrees = glm::degrees(eulerAngles.y);
                char labelBuffer3[128];
                
                snprintf(labelBuffer3, sizeof(labelBuffer3), "Y Rotation##%u", i);
                if (ImGui::SliderFloat(labelBuffer3, &yRotationDegrees, -90.0f, 90.0f, "%.1f°")) {
                    // Reconstruct matrix from components
                    eulerAngles.y = glm::radians(yRotationDegrees);
                    rotation = glm::quat(eulerAngles);

                    glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);
                    glm::mat4 R = glm::mat4_cast(rotation);
                    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

                    m.ModelMatrix() = T * R * S;
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

    void Application2::renderHDRControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("HDR Skybox Controls")) {
            ImGui::End();
            return;
        }

        // HDR Environment Controls
        if (ImGui::CollapsingHeader("HDR Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Environment Intensity", &this->forwardRenderer.getSkyOptionsUBO().environmentIntensity, 0.0f,
                10.0f, "%.2f");
        }

        // Environment Map Controls
        if (ImGui::CollapsingHeader("Environment Map", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Roughness Level", &this->forwardRenderer.getSkyOptionsUBO().roughnessLevel, 0.0f, 8.0f, "%.1f");

            bool useIrradiance = this->forwardRenderer.getSkyOptionsUBO().useIrradianceMap != 0;
            if (ImGui::Checkbox("Use Irradiance Map", &useIrradiance)) {
                this->forwardRenderer.getSkyOptionsUBO().useIrradianceMap = useIrradiance ? 1 : 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("?")) {
                // Optional: Add click action here if needed
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Toggle between prefiltered environment map (sharp reflections) and "
                    "irradiance map (diffuse lighting)");
            }
        }

        // Debug Visualization
        if (ImGui::CollapsingHeader("Debug Visualization")) {
            bool showMipLevels = this->forwardRenderer.getSkyOptionsUBO().showMipLevels != 0;
            if (ImGui::Checkbox("Show Mip Levels", &showMipLevels)) {
                this->forwardRenderer.getSkyOptionsUBO().showMipLevels = showMipLevels ? 1 : 0;
            }

            bool showCubeFaces = this->forwardRenderer.getSkyOptionsUBO().showCubeFaces != 0;
            if (ImGui::Checkbox("Show Cube Faces", &showCubeFaces)) {
                this->forwardRenderer.getSkyOptionsUBO().showCubeFaces = showCubeFaces ? 1 : 0;
            }
        }

        // Simplified Presets
        if (ImGui::CollapsingHeader("Presets")) {
            if (ImGui::Button("Default")) {
                this->forwardRenderer.getSkyOptionsUBO().environmentIntensity = 1.0f;
                this->forwardRenderer.getSkyOptionsUBO().roughnessLevel = 0.5f;
                this->forwardRenderer.getSkyOptionsUBO().useIrradianceMap = 0;
                this->forwardRenderer.getSkyOptionsUBO().showMipLevels = 0;
                this->forwardRenderer.getSkyOptionsUBO().showCubeFaces = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("High Exposure")) {
                this->forwardRenderer.getSkyOptionsUBO().environmentIntensity = 1.5f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Low Exposure")) {
                this->forwardRenderer.getSkyOptionsUBO().environmentIntensity = 0.8f;
            }

            if (ImGui::Button("Sharp Reflections")) {
                this->forwardRenderer.getSkyOptionsUBO().roughnessLevel = 0.0f;
                this->forwardRenderer.getSkyOptionsUBO().useIrradianceMap = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Diffuse Lighting")) {
                this->forwardRenderer.getSkyOptionsUBO().useIrradianceMap = 1;
            }
        }

        ImGui::End();
    }

    void Application2::initializeWithConfig(const ApplicationConfig& config)
    {
        // camea 설정은 추후에 구현
        this->loadModels(config.models);

        this->forwardRenderer.prepareForModels(
            this->models,
            this->swapChain->getSwapChainImageFormat(),
            this->cxt->getDepthStencil()->depthFormat,
            this->msaaSamples,
            this->extent.width,
            this->extent.height
        );
    }

    void Application2::loadModels(const std::vector<ModelConfig>& modelConfigs)
    {
        for (const auto& modelConfig : modelConfigs) {
            this->models.emplace_back(*cxt);
        auto& model = this->models.back();

        cString fullPath = this->RootPath + this->AssetsPath + modelConfig.filePath;
        model.loadFromModelFile(fullPath, modelConfig.isBistroObj);
        model.Name() = modelConfig.displayName;
        model.ModelMatrix() = modelConfig.transform;

        PRINT_TO_LOGGER("Loaded model '%s'\n", modelConfig.displayName.c_str());
        PRINT_TO_LOGGER("   - Meshes: %zu\n", model.Meshes().size());
        PRINT_TO_LOGGER("   - Materials: %zu\n", model.Materials().size());
        PRINT_TO_LOGGER("   - Textures: %zu\n", model.Textures().size());

        // Setup animation if model supports it
        if (model.hasAnimations() && modelConfig.autoPlayAnimation) {
            PRINT_TO_LOGGER("   Animations: %d found\n", model.getAnimationCount());

            if (model.getAnimationCount() > 0) {
                uint32_t animIndex = std::min(modelConfig.initialAnimationIndex, 
                                             model.getAnimationCount() - 1);
                model.setAnimationIndex(animIndex);
                model.setAnimationLooping(modelConfig.loopAnimation);
                model.setAnimationSpeed(modelConfig.animationSpeed);
                model.playAnimation();

                PRINT_TO_LOGGER("   ▶ Started animation: '%s' (%.2f sec)\n", 
                               model.getAnimation()->getCurrentAnimationName(),
                               model.getAnimation()->getDuration());
            }
        }
        else if (!model.hasAnimations()) {
            PRINT_TO_LOGGER("   Animations: None\n");
        }
        
        PRINT_TO_LOGGER("\n"); // 빈 줄
        }
    }
    
    void Application2::setupCallbacks()
    {
        // engine에서 제공하는 카메라 컨트롤 윈도우 사용
        this->window->setCamera(this->camera);
    }
    
    void Application2::initializeVulkanResources()
    {
        this->msaaSamples = helper::getMaxUsableSampleCount(this->cxt->getDevice()->physicalDevice);
        this->commandBuffers = this->cxt->createGrapicsCommandBufferHanders(this->kMaxFramesInFlight);
        this->camera = std::make_shared<vkengine::object::Camera2>();

        cFloat fov = camera->getFov();
        cFloat nearP = camera->getNearP();
        cFloat farP = camera->getFarP();
        cFloat aspectRatio = static_cast<cFloat>(this->extent.width) / static_cast<cFloat>(this->extent.height);
        camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);
        
        // engine에서 이미 fence와 semaphore를 생성하므로 생략
    }

    void Application2::renderCameraControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(10, 350), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Camera Controls")) {
            ImGui::End();
            return;
        }

        object::Camera2& camera = *this->camera;
        cVec3 CameraPosition = this->camera->getPos();
        cFloat Yaw = this->camera->getYaw();
        cFloat Pithch = this->camera->getPitch();

        // Camera Information Display
        if (ImGui::CollapsingHeader("Camera Information", ImGuiTreeNodeFlags_DefaultOpen)) {
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
        if (ImGui::CollapsingHeader("Position Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            glm::vec3 tempPosition = camera.getPos();
            if (ImGui::SliderFloat3("Position", &tempPosition.x, -50.0f, 50.0f, "%.2f")) {
                camera.setPos(tempPosition);
            }

            // Quick position buttons
            if (ImGui::Button("Reset Position")) {
                camera.setPos(glm::vec3(0.0f, 0.0f, -2.5f));
            }
            ImGui::SameLine();
            if (ImGui::Button("View Origin")) {
                camera.setPos(glm::vec3(0.0f, 0.0f, 5.0f));
            }
        }

        // Camera Rotation Controls
        if (ImGui::CollapsingHeader("Rotation Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            /*glm::vec3 tempRotation = camera.rotation;
            if (ImGui::SliderFloat3("Rotation (degrees)", &tempRotation.x, -180.0f, 180.0f, "%.1f°")) {
                camera_.setRotation(tempRotation);
            }*/

            // Quick rotation buttons
            if (ImGui::Button("Reset Rotation")) {
                camera.setPitch(cFloat(0.0f));
                camera.setYaw(cFloat(0.0f));
            }
            ImGui::SameLine();
        }

        //// Camera Settings
        //if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        //    // Movement and rotation speed controls
        //    float movementSpeed = camera_.movementSpeed;
        //    if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.1f, 50.0f, "%.1f")) {
        //        camera_.setMovementSpeed(movementSpeed);
        //    }

        //    float rotationSpeed = camera_.rotationSpeed;
        //    if (ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 0.01f, 2.0f, "%.2f")) {
        //        camera_.setRotationSpeed(rotationSpeed);
        //    }

        //    // Field of view control
        //    float currentFov = camera_.fov;
        //    if (ImGui::SliderFloat("Field of View", &currentFov, 30.0f, 120.0f, "%.1f°")) {
        //        const float aspectRatio = float(windowSize_.width) / windowSize_.height;
        //        camera_.setPerspective(currentFov, aspectRatio, camera_.znear, camera_.zfar);
        //    }

        //    // Near and far plane controls
        //    float nearPlane = camera_.znear;
        //    float farPlane = camera_.zfar;
        //    if (ImGui::SliderFloat("Near Plane", &nearPlane, 0.001f, 10.0f, "%.3f")) {
        //        const float aspectRatio = float(windowSize_.width) / windowSize_.height;
        //        camera_.setPerspective(camera_.fov, aspectRatio, nearPlane, camera_.zfar);
        //    }
        //    if (ImGui::SliderFloat("Far Plane", &farPlane, 10.0f, 10000.0f, "%.0f")) {
        //        const float aspectRatio = float(windowSize_.width) / windowSize_.height;
        //        camera_.setPerspective(camera_.fov, aspectRatio, camera_.znear, farPlane);
        //    }
        //}

        //// Camera Presets
        //if (ImGui::CollapsingHeader("Presets")) {
        //    if (ImGui::Button("Helmet View")) {
        //        camera_.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
        //        camera_.setRotation(glm::vec3(0.0f));
        //        camera_.type = hlab::Camera::CameraType::firstperson;
        //    }
        //    ImGui::SameLine();
        //    if (ImGui::Button("Side View")) {
        //        camera_.setPosition(glm::vec3(3.0f, 0.0f, 0.0f));
        //        camera_.setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
        //    }

        //    if (ImGui::Button("Top View")) {
        //        camera_.setPosition(glm::vec3(0.0f, 5.0f, 0.0f));
        //        camera_.setRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
        //    }
        //    ImGui::SameLine();
        //    if (ImGui::Button("Perspective View")) {
        //        camera_.setPosition(glm::vec3(2.0f, 2.0f, 2.0f));
        //        camera_.setRotation(glm::vec3(-25.0f, -45.0f, 0.0f));
        //    }
        //}

        // Controls Information
        if (ImGui::CollapsingHeader("Controls Help")) {
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

    void Application2::renderPostProcessingControlWindow()
    {
        ImGui::SetNextWindowPos(ImVec2(680, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Post-Processing Controls")) {
            ImGui::End();
            return;
        }

        // Tone Mapping Controls
        if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char* toneMappingNames[] = { "None",        "Reinhard",          "ACES",
                                              "Uncharted 2", "GT (Gran Turismo)", "Lottes",
                                              "Exponential", "Reinhard Extended", "Luminance",
                                              "Hable" };
            ImGui::Combo("Tone Mapping Type", &this->forwardRenderer.getPostProcessingOptionsUBO().toneMappingType,
                toneMappingNames, IM_ARRAYSIZE(toneMappingNames));

            ImGui::SliderFloat("Exposure", &this->forwardRenderer.getPostProcessingOptionsUBO().exposure, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("Gamma", &this->forwardRenderer.getPostProcessingOptionsUBO().gamma, 1.0f / 2.2f, 2.2f, "%.2f");

            if (this->forwardRenderer.getPostProcessingOptionsUBO().toneMappingType == 7) { // Reinhard Extended
                ImGui::SliderFloat("Max White", &this->forwardRenderer.getPostProcessingOptionsUBO().maxWhite, 1.0f, 20.0f,
                    "%.1f");
            }
        }

        // Color Grading Controls
        if (ImGui::CollapsingHeader("Color Grading", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Contrast", &this->forwardRenderer.getPostProcessingOptionsUBO().contrast, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("Brightness", &this->forwardRenderer.getPostProcessingOptionsUBO().brightness, -1.0f, 1.0f,
                "%.2f");
            ImGui::SliderFloat("Saturation", &this->forwardRenderer.getPostProcessingOptionsUBO().saturation, 0.0f, 2.0f, "%.2f");
            ImGui::SliderFloat("Vibrance", &this->forwardRenderer.getPostProcessingOptionsUBO().vibrance, -1.0f, 1.0f, "%.2f");
        }

        // Effects Controls
        if (ImGui::CollapsingHeader("Effects")) {
            ImGui::SliderFloat("Vignette Strength", &this->forwardRenderer.getPostProcessingOptionsUBO().vignetteStrength, 0.0f,
                1.0f, "%.2f");
            if (this->forwardRenderer.getPostProcessingOptionsUBO().vignetteStrength > 0.0f) {
                ImGui::SliderFloat("Vignette Radius", &this->forwardRenderer.getPostProcessingOptionsUBO().vignetteRadius, 0.1f,
                    1.5f, "%.2f");
            }

            ImGui::SliderFloat("Film Grain", &this->forwardRenderer.getPostProcessingOptionsUBO().filmGrainStrength, 0.0f, 0.2f,
                "%.3f");
            ImGui::SliderFloat("Chromatic Aberration", &this->forwardRenderer.getPostProcessingOptionsUBO().chromaticAberration,
                0.0f, 5.0f, "%.1f");
        }

        // Debug Controls
        if (ImGui::CollapsingHeader("Debug Visualization")) {
            const char* debugModeNames[] = { "Off", "Tone Mapping Comparison", "Color Channels",
                                            "Split Comparison" };
            ImGui::Combo("Debug Mode", &this->forwardRenderer.getPostProcessingOptionsUBO().debugMode, debugModeNames,
                IM_ARRAYSIZE(debugModeNames));

            if (this->forwardRenderer.getPostProcessingOptionsUBO().debugMode == 2) { // Color Channels
                const char* channelNames[] = { "All",       "Red Only", "Green Only",
                                              "Blue Only", "Alpha",    "Luminance" };
                ImGui::Combo("Show Channel", &this->forwardRenderer.getPostProcessingOptionsUBO().showOnlyChannel, channelNames,
                    IM_ARRAYSIZE(channelNames));
            }

            if (this->forwardRenderer.getPostProcessingOptionsUBO().debugMode == 3) { // Split Comparison
                ImGui::SliderFloat("Split Position", &this->forwardRenderer.getPostProcessingOptionsUBO().debugSplit, 0.0f, 1.0f,
                    "%.2f");
            }
        }

        // Presets
        if (ImGui::CollapsingHeader("Presets")) {
            if (ImGui::Button("Default")) {
                this->forwardRenderer.getPostProcessingOptionsUBO().toneMappingType = 2; // ACES
                this->forwardRenderer.getPostProcessingOptionsUBO().exposure = 1.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().gamma = 2.2f;
                this->forwardRenderer.getPostProcessingOptionsUBO().contrast = 1.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().brightness = 0.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().saturation = 1.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().vibrance = 0.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().vignetteStrength = 0.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().chromaticAberration = 0.0f;
                this->forwardRenderer.getPostProcessingOptionsUBO().debugMode = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cinematic")) {
                this->forwardRenderer.getPostProcessingOptionsUBO().toneMappingType = 3; // Uncharted 2
                this->forwardRenderer.getPostProcessingOptionsUBO().exposure = 1.2f;
                this->forwardRenderer.getPostProcessingOptionsUBO().contrast = 1.1f;
                this->forwardRenderer.getPostProcessingOptionsUBO().saturation = 0.9f;
                this->forwardRenderer.getPostProcessingOptionsUBO().vignetteStrength = 0.3f;
                this->forwardRenderer.getPostProcessingOptionsUBO().vignetteRadius = 0.8f;
                this->forwardRenderer.getPostProcessingOptionsUBO().filmGrainStrength = 0.02f;
            }

            if (ImGui::Button("High Contrast")) {
                this->forwardRenderer.getPostProcessingOptionsUBO().contrast = 1.5f;
                this->forwardRenderer.getPostProcessingOptionsUBO().brightness = 0.1f;
                this->forwardRenderer.getPostProcessingOptionsUBO().saturation = 1.3f;
                this->forwardRenderer.getPostProcessingOptionsUBO().vignetteStrength = 0.2f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Low Contrast")) {
                this->forwardRenderer.getPostProcessingOptionsUBO().contrast = 0.7f;
                this->forwardRenderer.getPostProcessingOptionsUBO().brightness = 0.05f;
                this->forwardRenderer.getPostProcessingOptionsUBO().saturation = 0.8f;
            }

            if (ImGui::Button("Show Tone Mapping")) {
                this->forwardRenderer.getPostProcessingOptionsUBO().debugMode = 1;
                this->forwardRenderer.getPostProcessingOptionsUBO().exposure = 2.0f;
            }
        }

        ImGui::End();
    }

}