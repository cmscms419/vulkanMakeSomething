#include "Application2.h"
#include "PostProcessing.h"

#include <thread>
#include <filesystem>
#include <stdexcept>

#define IMGUI_DEFINE_MATH_OPERATORS
using namespace vkengine; 
using namespace vkengine::gui;

PostProcessingExample::PostProcessingExample(std::string root_path, cBool useSwapchain)
    : 
    VulkanEngineWin2(useSwapchain),
    RootPath{ root_path.c_str()},
     shaderManager{*this->cxt,
                     RootPath + SHADER_PATH,
                     { 
                      {"gui", {"vertimgui.spv", "fragimgui.spv"}},
                      {"sky", {"vertskybox2.spv", "fragskybox2.spv"}},
                      {"post", {"vertpost.spv", "fragpost.spv"}}
                     }
    },
      guiRenderer{ *this->cxt, shaderManager, swapChain->getSwapChainImageFormat(), RootPath + RESOURSE_PATH },
      skyTextures{ *this->cxt },
      skyPipeline(*this->cxt, shaderManager),
      postPipeline(*this->cxt, shaderManager),
      hdrColorBuffer(*this->cxt),
      samplerLinearRepeat(*this->cxt),
      samplerLinearClamp(*this->cxt)
{
    PRINT_TO_LOGGER("Current working directory: %s", this->RootPath);
    
    this->commandBuffers = this->cxt->createGrapicsCommandBufferHanders(MAX_FRAMES_IN_FLIGHT);
    this->camera = std::make_shared<vkengine::object::Camera2>();
    this->window->setCamera(this->camera);

    // Initialize GUI
    guiRenderer.resize(this->extent.width, this->extent.height);

    samplerLinearRepeat.createLinearRepeat();
    samplerLinearClamp.createLinearClamp();

    cFloat aspectRatio = static_cast<cFloat>(this->extent.width) / static_cast<cFloat>(this->extent.height);

    cFloat fov = camera->getFov();
    cFloat nearP = camera->getNearP();
    cFloat farP = camera->getFarP();

    camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);

    // Initialize skybox and post-processing
    initializeSkybox();
    initializePostProcessing();
}

PostProcessingExample::~PostProcessingExample()
{
    vkDeviceWaitIdle(this->cxt->getDevice()->logicaldevice);
}

void PostProcessingExample::recreateSwapchain()
{
    VulkanEngineWin2::recreateSwapchain();

    guiRenderer.resize(this->extent.width, this->extent.height);
    
    this->samplerLinearClamp.createLinearClamp();

    hdrColorBuffer.createImage(
        this->extent.width,
        this->extent.height,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1, 1,
        (VkImageCreateFlagBits)0
    );

    hdrColorBuffer.setSampler(this->samplerLinearClamp.getSampler());

    postProcessingDescriptorSets.clear();
    postProcessingDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    //  4. Post-processing descriptor sets 재생성
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // 기존 descriptor set은 DescriptorSetHander가 자동으로 정리
        postProcessingDescriptorSets[i].create(
            *this->cxt,
            {
                hdrColorBuffer.ResourceBinding(),
                postProcessingOptionsUniforms[i].ResourceBinding()
            });
    }

    // projection 업데이트
    cFloat aspectRatio = static_cast<cFloat>(this->extent.width) / static_cast<cFloat>(this->extent.height);
    cFloat fov = camera->getFov();
    cFloat nearP = camera->getNearP();
    cFloat farP = camera->getFarP();
    camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);
}

void PostProcessingExample::initializeSkybox()
{

    // Set up HDR color buffer sampler
    hdrColorBuffer.createImage(this->extent.width,
        this->extent.height,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, (VkImageCreateFlagBits)0
    );

    // Create skybox pipeline (now renders to HDR buffer instead of swapChain)
    skyPipeline.createByName("sky", 
        VK_FORMAT_R16G16B16A16_SFLOAT, 
        cxt->getDepthStencil()->depthFormat,
        VK_SAMPLE_COUNT_1_BIT);

    // Load IBL textures
    std::string path = RootPath + Path + "resource/";
    skyTextures.LoadKTXMap(
        path + "cubeMap/specular_out.ktx2",
        path + "cubeMap/diffuse_out.ktx2",
        path + "cubeMap/outputLUT.png"
    );

    // Create uniform buffers for each frame
    sceneDataUniforms.clear();
    sceneDataUniforms.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        sceneDataUniforms.emplace_back(*this->cxt, sceneDataUBO);
    }

    // Create HDR sky options uniform buffers
    skyOptionsUniforms.clear();
    skyOptionsUniforms.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        skyOptionsUniforms.emplace_back(*this->cxt, skyOptionsUBO);
    }

    // Create descriptor sets for scene data and options (set 0)
    sceneDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        sceneDescriptorSets[i].create(*this->cxt,
            {
                sceneDataUniforms[i].ResourceBinding(), // binding 0
                skyOptionsUniforms[i].ResourceBinding() // binding 1
            });
    }

    // Create descriptor set for skybox textures (set 1)
    skyDescriptorSet.create(*this->cxt,
        {
            skyTextures.Prefiltered().getResourceBinding(),
            skyTextures.Irradiance().getResourceBinding(),
            skyTextures.BrdfLUT().getResourceBinding()
        });
}

void PostProcessingExample::initializePostProcessing()
{
    // Create post-processing pipeline
    postPipeline.createByName(
        "post", 
        swapChain->getSwapChainImageFormat(), 
        cxt->getDepthStencil()->depthFormat,
        VK_SAMPLE_COUNT_1_BIT);


    hdrColorBuffer.setSampler(this->samplerLinearClamp.getSampler());

    // Create post-processing uniform buffers for each frame
    postProcessingOptionsUniforms.clear();
    postProcessingOptionsUniforms.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        postProcessingOptionsUniforms.emplace_back(*this->cxt, postProcessingOptionsUBO);
    }

    // Create descriptor sets for post-processing (set 0: HDR texture + options uniform)
    postProcessingDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        postProcessingDescriptorSets[i].create(
            *this->cxt,
            {
                hdrColorBuffer.ResourceBinding(), // binding 0: sampler2D hdrColorBuffer
                postProcessingOptionsUniforms[i].ResourceBinding() // binding 1: PostProcessingOptions uniform
            });
    }
}

void PostProcessingExample::mainLoop()
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!window->shouldClose()) {
        window->pollEvents();

        // Calculate delta time for camera updates
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Clamp delta time to prevent large jumps
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

        // Update camera
         this->camera->update();

        // Update scene data UBO
        sceneDataUBO.projection = camera->getProjectionMatrix();
        sceneDataUBO.view = camera->getViewMatrix();
        sceneDataUBO.cameraPos = camera->getPos();

        updateGui(this->extent);
        guiRenderer.update();

        renderFrame();
    }

}

void PostProcessingExample::renderFrame()
{
    _VK_CHECK_RESULT_(vkWaitForFences(cxt->getDevice()->logicaldevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
    _VK_CHECK_RESULT_(vkResetFences(cxt->getDevice()->logicaldevice, 1, &inFlightFences[currentFrame]));

    // Update uniform buffers
    sceneDataUniforms[currentFrame].updateData();
    skyOptionsUniforms[currentFrame].updateData();
    postProcessingOptionsUniforms[currentFrame].updateData(); // Update post-processing options

    uint32_t imageIndex = 0;
    VkResult acquireResult =
        swapChain->acquireNextImage(presentSemaphores[currentSemaphore], imageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        this->recreateSwapchain();
        return;
    } else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        EXIT_TO_LOGGER("Failed to acquire swapChain image!");
    }

    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, extent);

    submitFrame(commandBuffers[currentFrame], presentSemaphores[currentSemaphore],
                renderSemaphores[currentSemaphore], inFlightFences[currentFrame]);

    // Present frame
    VkResult presentResult = swapChain->queuePresent(cxt->getDevice()->graphicsVKQueue, imageIndex,
                                                     renderSemaphores[currentSemaphore]);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        this->recreateSwapchain();
        return;
    }
    else if (presentResult == VK_SUBOPTIMAL_KHR) {
        EXIT_TO_LOGGER("Window resize not implemented");
    } else if (presentResult != VK_SUCCESS) {
        EXIT_TO_LOGGER("Failed to present swapChain image!");
    }

    this->caluCurrentValue();
}

void PostProcessingExample::updateGui(VkExtent2D windowSize)
{
    ImGuiIO& io = ImGui::GetIO();
    vkengine::platform::MouseState& mouseState = this->window->getmouseState();

    cVec3 CameraPosition = this->camera->getPos();
    cFloat Yaw = this->camera->getYaw();
    cFloat Pithch = this->camera->getPitch();

    // Update ImGui IO state
    io.DisplaySize = ImVec2(float(windowSize.width), float(windowSize.height));
    io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
    io.MouseDown[0] = mouseState.buttons.left;
    io.MouseDown[1] = mouseState.buttons.right;
    io.MouseDown[2] = mouseState.buttons.middle;

    // Begin GUI frame
    ImGui::NewFrame();

    // Camera info window
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Camera Control")) {
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", CameraPosition.x, CameraPosition.y, CameraPosition.z);
        ImGui::Text("Camera Yaw Pitch: (%.2f, %.2f)", Yaw, Pithch);

        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::Text("Mouse: Look around");
        ImGui::Text("WASD: Move");
        ImGui::Text("QE: Up/Down");

    }
    ImGui::End();

    // HDR Control window
    renderHDRControlWindow();

    // Post-processing Control window
    renderPostProcessingControlWindow();

    ImGui::Render();
}

void PostProcessingExample::renderHDRControlWindow()
{
    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("HDR Skybox Controls")) {
        ImGui::End();
        return;
    }

    // HDR Environment Controls
    if (ImGui::CollapsingHeader("HDR Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Environment Intensity", &skyOptionsUBO.environmentIntensity, 0.0f,
                           10.0f, "%.2f");
    }

    // Environment Map Controls
    if (ImGui::CollapsingHeader("Environment Map", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Roughness Level", &skyOptionsUBO.roughnessLevel, 0.0f, 8.0f, "%.1f");

        bool useIrradiance = skyOptionsUBO.useIrradianceMap != 0;
        if (ImGui::Checkbox("Use Irradiance Map", &useIrradiance)) {
            skyOptionsUBO.useIrradianceMap = useIrradiance ? 1 : 0;
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
        bool showMipLevels = skyOptionsUBO.showMipLevels != 0;
        if (ImGui::Checkbox("Show Mip Levels", &showMipLevels)) {
            skyOptionsUBO.showMipLevels = showMipLevels ? 1 : 0;
        }

        bool showCubeFaces = skyOptionsUBO.showCubeFaces != 0;
        if (ImGui::Checkbox("Show Cube Faces", &showCubeFaces)) {
            skyOptionsUBO.showCubeFaces = showCubeFaces ? 1 : 0;
        }
    }

    // Simplified Presets
    if (ImGui::CollapsingHeader("Presets")) {
        if (ImGui::Button("Default")) {
            skyOptionsUBO.environmentIntensity = 1.0f;
            skyOptionsUBO.roughnessLevel = 0.5f;
            skyOptionsUBO.useIrradianceMap = 0;
            skyOptionsUBO.showMipLevels = 0;
            skyOptionsUBO.showCubeFaces = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("High Exposure")) {
            skyOptionsUBO.environmentIntensity = 1.5f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Low Exposure")) {
            skyOptionsUBO.environmentIntensity = 0.8f;
        }

        if (ImGui::Button("Sharp Reflections")) {
            skyOptionsUBO.roughnessLevel = 0.0f;
            skyOptionsUBO.useIrradianceMap = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Diffuse Lighting")) {
            skyOptionsUBO.useIrradianceMap = 1;
        }
    }

    ImGui::End();
}

void PostProcessingExample::renderPostProcessingControlWindow()
{
    ImGui::SetNextWindowPos(ImVec2(680, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Post-Processing Controls")) {
        ImGui::End();
        return;
    }

    // Tone Mapping Controls
    if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* toneMappingNames[] = {"None",        "Reinhard",          "ACES",
                                          "Uncharted 2", "GT (Gran Turismo)", "Lottes",
                                          "Exponential", "Reinhard Extended", "Luminance",
                                          "Hable"};
        ImGui::Combo("Tone Mapping Type", &postProcessingOptionsUBO.toneMappingType,
                     toneMappingNames, IM_ARRAYSIZE(toneMappingNames));

        ImGui::SliderFloat("Exposure", &postProcessingOptionsUBO.exposure, 0.1f, 5.0f, "%.2f");
        ImGui::SliderFloat("Gamma", &postProcessingOptionsUBO.gamma, 1.0f / 2.2f, 2.2f, "%.2f");

        if (postProcessingOptionsUBO.toneMappingType == 7) { // Reinhard Extended
            ImGui::SliderFloat("Max White", &postProcessingOptionsUBO.maxWhite, 1.0f, 20.0f,
                               "%.1f");
        }
    }

    // Color Grading Controls
    if (ImGui::CollapsingHeader("Color Grading", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Contrast", &postProcessingOptionsUBO.contrast, 0.0f, 3.0f, "%.2f");
        ImGui::SliderFloat("Brightness", &postProcessingOptionsUBO.brightness, -1.0f, 1.0f,
                           "%.2f");
        ImGui::SliderFloat("Saturation", &postProcessingOptionsUBO.saturation, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Vibrance", &postProcessingOptionsUBO.vibrance, -1.0f, 1.0f, "%.2f");
    }

    // Effects Controls
    if (ImGui::CollapsingHeader("Effects")) {
        ImGui::SliderFloat("Vignette Strength", &postProcessingOptionsUBO.vignetteStrength, 0.0f,
                           1.0f, "%.2f");
        if (postProcessingOptionsUBO.vignetteStrength > 0.0f) {
            ImGui::SliderFloat("Vignette Radius", &postProcessingOptionsUBO.vignetteRadius, 0.1f,
                               1.5f, "%.2f");
        }

        ImGui::SliderFloat("Film Grain", &postProcessingOptionsUBO.filmGrainStrength, 0.0f, 0.2f,
                           "%.3f");
        ImGui::SliderFloat("Chromatic Aberration", &postProcessingOptionsUBO.chromaticAberration,
                           0.0f, 5.0f, "%.1f");
    }

    // Debug Controls
    if (ImGui::CollapsingHeader("Debug Visualization")) {
        const char* debugModeNames[] = {"Off", "Tone Mapping Comparison", "Color Channels",
                                        "Split Comparison"};
        ImGui::Combo("Debug Mode", &postProcessingOptionsUBO.debugMode, debugModeNames,
                     IM_ARRAYSIZE(debugModeNames));

        if (postProcessingOptionsUBO.debugMode == 2) { // Color Channels
            const char* channelNames[] = {"All",       "Red Only", "Green Only",
                                          "Blue Only", "Alpha",    "Luminance"};
            ImGui::Combo("Show Channel", &postProcessingOptionsUBO.showOnlyChannel, channelNames,
                         IM_ARRAYSIZE(channelNames));
        }

        if (postProcessingOptionsUBO.debugMode == 3) { // Split Comparison
            ImGui::SliderFloat("Split Position", &postProcessingOptionsUBO.debugSplit, 0.0f, 1.0f,
                               "%.2f");
        }
    }

    // Presets
    if (ImGui::CollapsingHeader("Presets")) {
        if (ImGui::Button("Default")) {
            postProcessingOptionsUBO.toneMappingType = 2; // ACES
            postProcessingOptionsUBO.exposure = 1.0f;
            postProcessingOptionsUBO.gamma = 2.2f;
            postProcessingOptionsUBO.contrast = 1.0f;
            postProcessingOptionsUBO.brightness = 0.0f;
            postProcessingOptionsUBO.saturation = 1.0f;
            postProcessingOptionsUBO.vibrance = 0.0f;
            postProcessingOptionsUBO.vignetteStrength = 0.0f;
            postProcessingOptionsUBO.filmGrainStrength = 0.0f;
            postProcessingOptionsUBO.chromaticAberration = 0.0f;
            postProcessingOptionsUBO.debugMode = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cinematic")) {
            postProcessingOptionsUBO.toneMappingType = 3; // Uncharted 2
            postProcessingOptionsUBO.exposure = 1.2f;
            postProcessingOptionsUBO.contrast = 1.1f;
            postProcessingOptionsUBO.saturation = 0.9f;
            postProcessingOptionsUBO.vignetteStrength = 0.3f;
            postProcessingOptionsUBO.vignetteRadius = 0.8f;
            postProcessingOptionsUBO.filmGrainStrength = 0.02f;
        }

        if (ImGui::Button("High Contrast")) {
            postProcessingOptionsUBO.contrast = 1.5f;
            postProcessingOptionsUBO.brightness = 0.1f;
            postProcessingOptionsUBO.saturation = 1.3f;
            postProcessingOptionsUBO.vignetteStrength = 0.2f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Low Contrast")) {
            postProcessingOptionsUBO.contrast = 0.7f;
            postProcessingOptionsUBO.brightness = 0.05f;
            postProcessingOptionsUBO.saturation = 0.8f;
        }

        if (ImGui::Button("Show Tone Mapping")) {
            postProcessingOptionsUBO.debugMode = 1;
            postProcessingOptionsUBO.exposure = 2.0f;
        }
    }

    ImGui::End();
}

void PostProcessingExample::recordCommandBuffer(VKCommandBufferHander& cmd, uint32_t imageIndex,
                                                     VkExtent2D windowSize)
{
    vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
    VkCommandBufferBeginInfo cmdBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));

    // === PASS 1: Render skybox to HDR color buffer ===
    
    // Transition HDR color buffer to color attachment
    hdrColorBuffer.transitionTo(
        cmd.getCommandBuffer(), 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    // HDR skybox rendering pass
    VkClearColorValue hdrClearColorValue = {{0.0f, 0.0f, 0.0f, 1.0f}}; // Black clear for HDR

    VkRenderingAttachmentInfo hdrColorAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    hdrColorAttachment.imageView = hdrColorBuffer.getImageView();
    hdrColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    hdrColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    hdrColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    hdrColorAttachment.clearValue.color = hdrClearColorValue;

    VkRenderingInfo hdrRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO};
    hdrRenderingInfo.renderArea = {0, 0, windowSize.width, windowSize.height};
    hdrRenderingInfo.layerCount = 1;
    hdrRenderingInfo.colorAttachmentCount = 1;
    hdrRenderingInfo.pColorAttachments = &hdrColorAttachment;

    vkCmdBeginRendering(cmd.getCommandBuffer(), &hdrRenderingInfo);

    // Set viewport and scissor
    VkViewport viewport{0.0f, 0.0f, (float)windowSize.width, (float)windowSize.height, 0.0f, 1.0f};
    VkRect2D scissor{0, 0, windowSize.width, windowSize.height};
    vkCmdSetViewport(cmd.getCommandBuffer(), 0, 1, &viewport);
    vkCmdSetScissor(cmd.getCommandBuffer(), 0, 1, &scissor);

    // Render skybox to HDR buffer
    vkCmdBindPipeline(cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipeline.getPipeline());

    const auto skyDescriptorSets = std::vector{ sceneDescriptorSets[currentFrame].get(), skyDescriptorSet.get() };

    vkCmdBindDescriptorSets(
        cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, skyPipeline.getPipelineLayout(), 0,
        static_cast<cUint32_t>(skyDescriptorSets.size()), skyDescriptorSets.data(), 0, nullptr);

    // Draw skybox - 36 vertices from hardcoded data in shader
    vkCmdDraw(cmd.getCommandBuffer(), 36, 1, 0, 0);

    vkCmdEndRendering(cmd.getCommandBuffer());

    // === PASS 2: Post-process HDR buffer to swapChain ===

    // Transition HDR color buffer to shader read
    hdrColorBuffer.transitionTo(
        cmd.getCommandBuffer(), 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_2_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

    // Transition swapChain image to color attachment
    swapChain->transitionTo(
        cmd.getCommandBuffer(),
        imageIndex,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    // Post-processing rendering pass
    VkClearColorValue postClearColorValue = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingAttachmentInfo postColorAttachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    postColorAttachment.imageView = swapChain->getSwapChainImageView(imageIndex);
    postColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    postColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    postColorAttachment.clearValue.color = postClearColorValue;

    VkRenderingInfo postRenderingInfo{VK_STRUCTURE_TYPE_RENDERING_INFO};
    postRenderingInfo.renderArea = {0, 0, windowSize.width, windowSize.height};
    postRenderingInfo.layerCount = 1;
    postRenderingInfo.colorAttachmentCount = 1;
    postRenderingInfo.pColorAttachments = &postColorAttachment;

    vkCmdBeginRendering(cmd.getCommandBuffer(), &postRenderingInfo);

    vkCmdSetViewport(cmd.getCommandBuffer(), 0, 1, &viewport);
    vkCmdSetScissor(cmd.getCommandBuffer(), 0, 1, &scissor);

    // Render post-processing fullscreen quad
    vkCmdBindPipeline(cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.getPipeline());

    const auto postDescriptorSets = std::vector{ postProcessingDescriptorSets[currentFrame].get() };

    vkCmdBindDescriptorSets(
        cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.getPipelineLayout(), 0,
        static_cast<uint32_t>(postDescriptorSets.size()), postDescriptorSets.data(), 0, nullptr);

    // Draw fullscreen quad - 6 vertices from hardcoded data in post.vert
    vkCmdDraw(cmd.getCommandBuffer(), 6, 1, 0, 0);

    vkCmdEndRendering(cmd.getCommandBuffer());

    // === PASS 3: Draw GUI on top ===

    // Draw GUI on top of the post-processed image
    guiRenderer.draw(cmd.getCommandBuffer(), swapChain->getSwapChainImageView(imageIndex), viewport);

    // Transition swapChain image to present
    swapChain->transitionTo(
        cmd.getCommandBuffer(), 
        imageIndex,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_2_NONE, 
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

    _VK_CHECK_RESULT_(vkEndCommandBuffer(cmd.getCommandBuffer()));
}

void PostProcessingExample::submitFrame(VKCommandBufferHander& commandBuffer,
                                             VkSemaphore waitSemaphore, VkSemaphore signalSemaphore,
                                             VkFence fence)
{
    VkSemaphoreSubmitInfo waitSemaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    waitSemaphoreInfo.semaphore = waitSemaphore;
    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    waitSemaphoreInfo.value = 0;
    waitSemaphoreInfo.deviceIndex = 0;

    VkSemaphoreSubmitInfo signalSemaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    signalSemaphoreInfo.semaphore = signalSemaphore;
    signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    signalSemaphoreInfo.value = 0;
    signalSemaphoreInfo.deviceIndex = 0;

    VkCommandBufferSubmitInfo cmdBufferInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO};
    cmdBufferInfo.commandBuffer = commandBuffer.getCommandBuffer();
    cmdBufferInfo.deviceMask = 0;

    VkSubmitInfo2 submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

    _VK_CHECK_RESULT_(vkQueueSubmit2(commandBuffer.getQueue(), 1, &submitInfo, fence));
}
