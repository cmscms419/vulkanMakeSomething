#include "common.h"

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
#include "VKgui.h"

#include "log.h"

using namespace vkengine;
using namespace vkengine::Log;

// Forward declarations
void renderColorControlWindow();
void renderColorPresets();

// Global variables
glm::vec4 clearColor = glm::vec4(0.2f, 0.3f, 0.5f, 1.0f); // Default blue-ish color with alpha

// Separate preset rendering for clarity
void renderColorPresets()
{
    const struct ColorPreset
    {
        const char* name;
        glm::vec4 color;
    } presets[] = { {"Sky Blue", {0.53f, 0.81f, 0.92f, 1.0f}},
                   {"Sunset", {1.0f, 0.65f, 0.0f, 1.0f}},
                   {"Night", {0.05f, 0.05f, 0.15f, 1.0f}},
                   {"Forest", {0.13f, 0.55f, 0.13f, 1.0f}},
                   {"Reset", {0.2f, 0.3f, 0.5f, 1.0f}} };

    constexpr int buttonsPerRow = 2;
    for (int i = 0; i < std::size(presets); ++i) {
        if (ImGui::Button(presets[i].name)) {
            clearColor = presets[i].color;
        }

        if ((i + 1) % buttonsPerRow != 0 && i < std::size(presets) - 1) {
            ImGui::SameLine();
        }
    }
}

// Enhanced GUI function with better structure
void updateGui(VkExtent2D windowSize, vkengine::platform::MouseState& state)
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

    // Render color control window
    renderColorControlWindow();

    ImGui::Render();
}

// Separate the GUI window rendering for better organization
void renderColorControlWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Clear Color Control")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Control the background clear color:");
    ImGui::Separator();

    // Color picker (using ColorEdit4 to include alpha channel)
    ImGui::ColorEdit4("Clear Color", &clearColor.r);

    ImGui::Separator();
    ImGui::Text("Individual Controls:");

    // RGBA sliders
    ImGui::SliderFloat("Red", &clearColor.r, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Green", &clearColor.g, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Blue", &clearColor.b, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Alpha", &clearColor.a, 0.0f, 1.0f, "%.3f");

    ImGui::Separator();
    ImGui::Text("Color Preview:");

    // Preview button
    ImGui::ColorButton("Preview", ImVec4(clearColor.r, clearColor.g, clearColor.b, clearColor.a), 0,
        ImVec2(50, 50));

    ImGui::Separator();
    ImGui::Text("Presets:");

    // Preset buttons with better layout
    renderColorPresets();

    ImGui::End();
}

void recordCommanBuffer(
    VKCommandBufferHander& cmd, VKSwapChain& swapchain, cUint32_t imageIndex,
    VkExtent2D windowSize, gui::VKimguiRenderer& guiRenderer)
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

    // Use clearColor directly - no need for conversion function
    VkClearColorValue clearColorValue = { {clearColor.r, clearColor.g, clearColor.b, clearColor.a} };

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
    vkCmdEndRendering(cmd.getCommandBuffer());

    // Draw GUI on top of the clear color
    VkViewport viewport{ 0.0f, 0.0f, (float)windowSize.width, (float)windowSize.height, 0.0f, 1.0f };
    guiRenderer.draw(cmd.getCommandBuffer(), swapchain.ImageView(imageIndex), viewport);

    // COLOR_ATTACHMENT_OPTIMAL에서 PRESENT_SRC_KHR로 전환
    swapchain.getBarrierHelper(imageIndex).transitionImageLayout2(
        cmd.getCommandBuffer(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_2_NONE,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

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
    createInfo.width = 1280;
    createInfo.height = 720;
    createInfo.title = "Vulkan Engine Window";

    std::unique_ptr<platform::VirtualWindows> window = 
        platform::WindowFactory::create(platform::WindowFactory::Backend::GLFW);
    
    //window->setKeyCallback(keyCallbackWrapper);
    //window->setMouseButtonCallback();
    //window->setMouseMoveCallback();
    
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
        { {"gui", {"vertimgui.spv", "fragimgui.spv"}} }
    );

    gui::VKimguiRenderer guiRenderer(vkContext, shaderManager, swapChain.getSwapChainImageFormat());

    std::vector<VKCommandBufferHander> commandBuffers = vkContext.createGrapicsCommandBufferHanders(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> presentSemaphores;
    std::vector<VkSemaphore> renderSemaphores;
    std::vector<VkFence> inFlightFences;

    helper::initializeSynchronization
    (
        vkContext.getDevice()->logicaldevice,
        MAX_FRAMES_IN_FLIGHT,
        swapChain.getSwapChainImageCount(),
        presentSemaphores,
        renderSemaphores,
        inFlightFences
    );

    cUint32_t currentFrame = 0;
    cUint32_t currentSemaphore = 0;

    // GUI 초기화
    guiRenderer.resize(extent.width, extent.height);
    
    while (!window->shouldClose())
    {
        window->pollEvents();

        updateGui(extent, window->getmouseState());

        guiRenderer.update();

        _VK_CHECK_RESULT_(vkWaitForFences(vkContext.getDevice()->logicaldevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(vkContext.getDevice()->logicaldevice, 1, &inFlightFences[currentFrame]));

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

        recordCommanBuffer(commandBuffers[currentFrame], swapChain, imageIndex, extent, guiRenderer);

        submitFrame(commandBuffers[currentFrame], presentSemaphores[currentSemaphore],
            renderSemaphores[currentSemaphore], inFlightFences[currentFrame]);

        // window에 출력해주는 frame
        VkResult presentResult = swapChain.queuePresent(
            vkContext.getDevice()->graphicsVKQueue, imageIndex,
            renderSemaphores[currentSemaphore]);

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            EXIT_TO_LOGGER("Window resize not implemented");
        }
        else if (presentResult != VK_SUCCESS) {
            EXIT_TO_LOGGER("Failed to present swapchain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        currentSemaphore = (currentSemaphore + 1) % swapChain.ImageCount();
    }

    vkContext.waitGraphicsQueueIdle();

    helper::cleanupSynchronization(vkContext.getDevice()->logicaldevice, presentSemaphores, renderSemaphores, inFlightFences);

    return 0;
}