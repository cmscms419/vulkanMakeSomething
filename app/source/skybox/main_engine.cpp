#include "VKContext.h"
#include "VKImage2D.h"
#include "VKShaderManager.h"
#include "VKshader.h"
#include "pipeLineHandle.h"
#include "VKbarrier2.h"
#include "VKCommadBufferHander.h"
#include "VKbuffer2.h"
#include "VKwindow.h"
#include "VKswapchain.h"
#include "Camera2.h"
#include "VKgui.h"
#include "VKSkyTexture.h"
#include "VKUniformBuffer2.h"
#include "DescriptorSet.h"

#include <chrono>

#include "log.h"

using namespace vkengine;
using namespace vkengine::Log;

using namespace vkengine;

// Scene data UBO structure matching skybox.vert
struct SceneDataUBO
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 cameraPos;
    float padding1;
    glm::vec3 directionalLightDir{ -1.0f, -1.0f, -1.0f };
    float padding2;
    glm::vec3 directionalLightColor{ 1.0f, 1.0f, 1.0f };
    float padding3;
    glm::mat4 lightSpaceMatrix{ 1.0f };
};

// HDR skybox-specific control options
struct SkyOptionsUBO
{
    // HDR Environment mapping controls
    float environmentIntensity = 1.0f; // Environment map intensity multiplier
    float roughnessLevel = 0.5f;       // Mip level for prefiltered map (0.0 = sharpest)
    uint32_t useIrradianceMap = 0;     // 0 = use prefiltered, 1 = use irradiance

    // Skybox visualization and debug
    uint32_t showMipLevels = 0; // Visualize mip levels as colors
    uint32_t showCubeFaces = 0; // Visualize cube faces as colors
    float padding1;
    float padding2;
    float padding3;
};

SceneDataUBO sceneDataUBO = {};
SkyOptionsUBO skyOptionsUBO = {};

std::vector<VKUniformBuffer2<SceneDataUBO>> sceneDataUniforms;
std::vector<VKUniformBuffer2<SkyOptionsUBO>> skyOptionsUniforms;
std::vector<DescriptorSetHander> sceneDescriptorSets;
DescriptorSetHander skyDescriptorSet;
bool shouldClose{ false };

cUint32_t currentFrame = 0;
cUint32_t currentSemaphore = 0;

void renderHDRControlWindow()
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

void updateGui(VkExtent2D windowSize, vkengine::platform::MouseState& state, vkengine::object::Camera2* camera)
{
    ImGuiIO& io = ImGui::GetIO();

    // Update ImGui IO state
    io.DisplaySize = ImVec2(float(windowSize.width), float(windowSize.height));
    io.MousePos = ImVec2(state.position.x, state.position.y);
    io.MouseDown[0] = state.buttons.left;
    io.MouseDown[1] = state.buttons.right;
    io.MouseDown[2] = state.buttons.middle;

    // Begin GUI frame
    ImGui::NewFrame();

    // Camera info window
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Camera Control")) {
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera->getPos().x, camera->getPos().y,
            camera->getPos().z);
        ImGui::Text("Camera Yaw Pitch: (%.2f, %.2f)", camera->getYaw(), camera->getPitch());

        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::Text("Mouse: Look around");
        ImGui::Text("WASD: Move");
        ImGui::Text("QE: Up/Down");
        ImGui::Text("F2: Toggle camera mode");

        //bool isFirstPerson = camera->type == hlab::Camera::CameraType::firstperson;
        //if (ImGui::Checkbox("First Person Mode", &isFirstPerson)) {
        //    camera->type = isFirstPerson ? hlab::Camera::CameraType::firstperson
        //        : hlab::Camera::CameraType::lookat;
        //}
    }
    ImGui::End();

    // HDR Control window
    renderHDRControlWindow();

    ImGui::Render();
}

void renderColorControlWindow()
{

}

void recordCommandBuffer(
    VKCommandBufferHander& cmd, VKSwapChain& swapchain, cUint32_t imageIndex,
    VkExtent2D windowSize, gui::VKimguiRenderer& guiRenderer, PipeLineHandle& skypipeline)
{
    vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
    VkCommandBufferBeginInfo cmdBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));

    swapchain.getBarrierHelper(imageIndex).transitionImageLayout2(
        cmd.getCommandBuffer(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    // Use fixed sky blue clear color
    VkClearColorValue clearColorValue = { {0.53f, 0.81f, 0.92f, 1.0f} };

    VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    colorAttachment.imageView = swapchain.ImageView(imageIndex);
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = clearColorValue;

    VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderingInfo.renderArea = { 0, 0, windowSize.width, windowSize.height };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd.getCommandBuffer(), &renderingInfo);

    VkViewport viewport{ 0.0f, 0.0f, (float)windowSize.width, (float)windowSize.height, 0.0f, 1.0f };
    VkRect2D scissor{ 0, 0, windowSize.width, windowSize.height };

    vkCmdSetViewport(cmd.getCommandBuffer(), 0, 1, &viewport);
    vkCmdSetScissor(cmd.getCommandBuffer(), 0, 1, &scissor);

    vkCmdBindPipeline(cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, skypipeline.getPipeline());

    const auto descriptorSets = std::vector{ sceneDescriptorSets[currentFrame].get(), skyDescriptorSet.get() };

    vkCmdBindDescriptorSets(cmd.getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, skypipeline.getPipelineLayout(), 
        0, static_cast<cUint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);

    vkCmdDraw(cmd.getCommandBuffer(), 36, 1, 0, 0);

    vkCmdEndRendering(cmd.getCommandBuffer());

    // Draw GUI on top of the skybox
    guiRenderer.draw(cmd.getCommandBuffer(), swapchain.getSwapChainImageView(imageIndex), viewport);

    swapchain.getBarrierHelper(imageIndex).transitionImageLayout2
        (
            cmd.getCommandBuffer(), 
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_NONE, 
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
        );

    _VK_CHECK_RESULT_(vkEndCommandBuffer(cmd.getCommandBuffer()));
}

void submitFrame(VKCommandBufferHander& commandBuffer, VkSemaphore waitSemaphore,
    VkSemaphore signalSemaphore, VkFence fence)
{
    VkSemaphoreSubmitInfo waitSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    waitSemaphoreInfo.semaphore = waitSemaphore;
    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    waitSemaphoreInfo.value = 0;
    waitSemaphoreInfo.deviceIndex = 0;

    VkSemaphoreSubmitInfo signalSemaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    signalSemaphoreInfo.semaphore = signalSemaphore;
    signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    signalSemaphoreInfo.value = 0;
    signalSemaphoreInfo.deviceIndex = 0;

    VkCommandBufferSubmitInfo cmdBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    cmdBufferInfo.commandBuffer = commandBuffer.getCommandBuffer();
    cmdBufferInfo.deviceMask = 0;

    VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;

    _VK_CHECK_RESULT_(vkQueueSubmit2(commandBuffer.getQueue(), 1, &submitInfo, fence));
}

int main(int argc, char* argv[]) {

    platform::WindowCreateInfo createInfo;
    createInfo.width = 1920;
    createInfo.height = 1080;
    createInfo.title = "Vulkan Engine Window";
    std::shared_ptr<vkengine::object::Camera2> camera = std::make_shared<vkengine::object::Camera2>();


    std::unique_ptr<platform::VirtualWindows> window = platform::WindowFactory::create(
        platform::WindowFactory::Backend::GLFW,
        createInfo,
        camera);

    int windowWidth = 0;
    int windowHeight = 0;

    window->getFramebufferSize(&windowWidth, &windowHeight);
    VKcontext vkContext(window->getRequiredExtensions(), true);

    VkExtent2D extent{ static_cast<cUint32_t>(windowWidth), static_cast<cUint32_t>(windowHeight) };

    VKSwapChain swapChain(vkContext, window->createVulkanSurface(vkContext.getInstance()), extent, true);


    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        EXIT_TO_LOGGER("경로를 가져오는 데 실패했습니다.");
    }

    VKShaderManager shaderManager
    (
        vkContext,
        root_path + "../../../../../../shader/",
        { 
          {"gui", {"vertimgui.spv", "fragimgui.spv"}} , 
          {"sky", {"vertskybox2.spv", "fragskybox2.spv"}} 
        }
    );
    gui::VKimguiRenderer guiRenderer(vkContext, shaderManager, swapChain.getSwapChainImageFormat());
    std::vector<VKCommandBufferHander> commandBuffers = vkContext.createGrapicsCommandBufferHanders(MAX_FRAMES_IN_FLIGHT);
    uint32_t imageCount = swapChain.getSwapChainImageCount();
    std::vector<VkSemaphore> presentSemaphores;
    std::vector<VkSemaphore> renderSemaphores;
    std::vector<VkFence> inFlightFences;

    PipeLineHandle skyboxpipeline(vkContext, shaderManager);
    skyboxpipeline.createByName("sky", swapChain.getSwapChainImageFormat(), vkContext.getDepthStencil()->depthFormat,
        VK_SAMPLE_COUNT_1_BIT);
    guiRenderer.resize(extent.width, extent.height);
    
    VKskyTexture skymap(vkContext);
    skymap.LoadKTXMap(
        root_path + "../../../../../../resource/cubeMap/specular_out.ktx2",
        root_path + "../../../../../../resource/cubeMap/diffuse_out.ktx2",
        root_path + "../../../../../../resource/cubeMap/outputLUT.png");

    helper::initializeSynchronization
    (
        vkContext.getDevice()->logicaldevice,
        MAX_FRAMES_IN_FLIGHT,
        swapChain.getSwapChainImageCount(),
        presentSemaphores,
        renderSemaphores,
        inFlightFences
    );



    // GUI 초기화
    guiRenderer.resize(extent.width, extent.height);

    camera->update();
    VkExtent2D extext = swapChain.getWindowSize();

    float aspectRatio = static_cast<float>(extext.width) / static_cast<float>(extext.height);

    float fov = camera->getFov();
    float nearP = camera->getNearP();
    float farP = camera->getFarP();

    camera->setPerspectiveProjection(fov, aspectRatio, nearP, farP);


    // Create uniform buffers for each frame
    sceneDataUniforms.clear();
    sceneDataUniforms.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        sceneDataUniforms.emplace_back(vkContext, sceneDataUBO);
    }

    // Create HDR sky options uniform buffers
    skyOptionsUniforms.clear();
    skyOptionsUniforms.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        skyOptionsUniforms.emplace_back(vkContext, skyOptionsUBO);
    }

    // Create descriptor sets for scene data and options (set 0)
    sceneDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        sceneDescriptorSets[i].create(vkContext,
            {
                sceneDataUniforms[i].ResourceBinding(), // binding 0
                skyOptionsUniforms[i].ResourceBinding() // binding 1
            });
    }

    // Create descriptor set for skybox textures (set 1)
    skyDescriptorSet.create(vkContext, 
        { 
            skymap.Prefiltered().getResourceBinding(),
            skymap.Irradiance().getResourceBinding(),
            skymap.BrdfLUT().getResourceBinding() 
        });

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!window->shouldClose() && !shouldClose)
    {
        window->pollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Clamp delta time to prevent large jumps
        deltaTime = std::min(deltaTime, 0.033f); // Max 33ms (30 FPS minimum)

        // camera update
        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_W])
        {
            camera->MoveForward(0.003);
        }

        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_S])
        {
            camera->MoveForward(-0.003);
        }

        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_A])
        {
            camera->MoveRight(-0.003);
        }

        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_D])
        {
            camera->MoveRight(0.003);
        }

        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_Q])
        {
            camera->MoveUp(-0.003);
        }

        if (window->getKeyBoardState().m_keyPressed[GLFW_KEY_E])
        {
            camera->MoveUp(0.003);
        }

        camera->update();

        sceneDataUBO.projection = camera->getProjectionMatrix();
        sceneDataUBO.view = camera->getViewMatrix();
        sceneDataUBO.cameraPos = camera->getPos();

        updateGui(extent, window->getmouseState(), camera.get());
        guiRenderer.update();

        _VK_CHECK_RESULT_(vkWaitForFences(vkContext.getDevice()->logicaldevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(vkContext.getDevice()->logicaldevice, 1, &inFlightFences[currentFrame]));

        sceneDataUniforms[currentFrame].updateData();
        skyOptionsUniforms[currentFrame].updateData();

        uint32_t imageIndex = 0;
        VkResult acquireResult = swapChain.acquireNextImage(presentSemaphores[currentSemaphore], imageIndex);

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            //this->recreateSwapChain(); // 스왑 체인을 다시 생성합니다.
            // TODO 다시 만들어야 한다. 
            EXIT_TO_LOGGER("Window resize not implemented\n");
        }
        else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            EXIT_TO_LOGGER("Failed to acquire swapchain image!");
        }
        else {
            _VK_CHECK_RESULT_(acquireResult);
        }

        recordCommandBuffer(commandBuffers[currentFrame], swapChain, imageIndex, extent, guiRenderer, skyboxpipeline);

        submitFrame(commandBuffers[currentFrame], presentSemaphores[currentSemaphore],
            renderSemaphores[currentSemaphore], inFlightFences[currentFrame]);
        
        VkResult presentResult = swapChain.queuePresent(vkContext.grapicsQueue(), imageIndex,
            renderSemaphores[currentSemaphore]);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            EXIT_TO_LOGGER("Window resize not implemented\n");
        }
        else if (presentResult != VK_SUCCESS) {
            EXIT_TO_LOGGER("Failed to present swapchain image!\n");
        }

        currentFrame = (currentFrame + 1) % 2;
        currentSemaphore = (currentSemaphore + 1) % swapChain.getSwapChainImageCount();

    }


    return 0;
}