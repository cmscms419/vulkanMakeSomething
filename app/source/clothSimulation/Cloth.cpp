#include "Cloth.h"

#include "log.h"

#include <chrono>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// 카메라 튜닝 값
//
// 천은 32 x 0.05 = 1.6m 크기이고 origin (0, 2, 0) 에 매달린다.
// sponza 데모와 달리 장면이 미터 단위로 작으므로 속도와 near/far 를 줄인다.
// ---------------------------------------------------------------------------
static constexpr cFloat CAMERA_BASE_SPEED = 2.0f;
static constexpr cFloat CAMERA_BOOST_MUL = 5.0f;
static constexpr cFloat CAMERA_SLOW_MUL = 0.2f;

static constexpr cFloat LOOK_SENSITIVITY = 2.0f;
static constexpr cFloat INVERT_X = -1.0f;
static constexpr cFloat INVERT_Y = -1.0f;

// yaw = pitch = 0 이면 시선은 (0, 0, +1) 이다.
// Camera2::setFreeCameraRotateDeltaRotation() 이 (0,0,-1) 이 아니라
// 멤버 Zaxis = (0, 0, 1) 을 회전시키기 때문이다. 헷갈리기 쉬운 부분이다.
// 그래서 천(z = 0) 을 보려면 카메라를 -z 쪽에 두어야 한다.
#define CAMERA_START_POSITION cVec3(0.8f, 1.2f, -4.0f)
static constexpr cFloat CAMERA_START_YAW = 0.0f;
static constexpr cFloat CAMERA_START_PITCH = 0.0f;

static constexpr cFloat CAMERA_NEAR_PLANE = 0.05f;
static constexpr cFloat CAMERA_FAR_PLANE = 500.0f;

namespace vkengine
{
    Cloth::Cloth(cString root_path)
        : Application(root_path), clothData(*this->cxt, this->shaderManager)
    {
        this->initCamera();
        this->initCloth();
    }

    Cloth::Cloth(const ApplicationConfig& config, cString root_path)
        : Application(config, root_path), clothData(*this->cxt, this->shaderManager)
    {
        this->initCamera();
        this->initCloth();
    }

    Cloth::~Cloth()
    {
    }

    void Cloth::initCamera()
    {
        camera->setCameraType(object::CameraType::FreeCamera);
        camera->setPos(CAMERA_START_POSITION);
        camera->setYaw(CAMERA_START_YAW);
        camera->setPitch(CAMERA_START_PITCH);

        // yaw/pitch 로부터 dir/right 를 다시 만들기 위해 0 회전을 한 번 적용한다.
        camera->rotateDeltaRotation(cVec3(0.0f));

        camera->setPerspectiveProjection(camera->getFov(),
                                         camera->getAspect(),
                                         CAMERA_NEAR_PLANE,
                                         CAMERA_FAR_PLANE);
        camera->update();
    }

    void Cloth::initCloth()
    {
        ClothSimulation::Info info{};
        info.gridX = 24;
        info.gridY = 24;
        info.spacing = 0.05f;
        info.mass = 0.01f;
        info.kStretch = 5000.0f;
        info.kShear = 2000.0f;
        info.kBend = 50.0f;
        info.useShear = true;
        info.useBend = true;
        info.pinTopCorners = true;
        info.origin = cVec3(0.0f, 2.0f, 0.0f);
        info.gravity = cVec3(0.0f, -9.8f, 0.0f);
        info.timeStep = 1.0f / 60.0f;
        info.iterations = 10;
        info.floorY = 0.0f;
        info.initialJitterZ = 0.0f;

        this->rebuildCloth(info);

        // 천 패스가 그릴 대상을 렌더러에 알려 준다.
        // 렌더 그래프는 Application 생성자에서 이미 만들어졌으므로 지금 넘겨야 한다.
        this->renderer.setClothData(&this->clothData);
    }

    void Cloth::rebuildCloth(const ClothSimulation::Info& info)
    {
        // GPU 가 이전 버퍼를 아직 읽고 있을 수 있다.
        // ClothDataManagement::create() 는 버퍼를 지우고 다시 만들므로 먼저 기다려야 한다.
        if (this->clothData.isReady())
        {
            this->cxt->waitIdle();
        }

        this->cloth.create(info);
        this->cloth.logSummary();

        this->clothData.create(this->cloth);

        this->pendingInfo = info;
        this->clothDirty = false;

        // create() 가 info 의 값으로 되돌려 놓으므로 한 번 더 밀어넣어
        // pendingInfo 와 clothData 를 일치시킨다.
        this->applyLiveParams();

        this->hasReadback = false;
        this->readbackVertices.clear();
    }

    // 재빌드가 필요 없는 값만 clothData 에 전달한다.
    //
    // 형상과 재질(gridX, spacing, mass, k...)은 정점/스프링 버퍼의 내용을
    // 바꾸므로 Apply 로 재빌드해야 하지만, 아래 네 개는 그렇지 않다.
    //   iterations  step() 의 루프 횟수             - CPU 멤버 하나
    //   floorY      clothFinalize.comp 의 푸시 상수  - CPU 멤버 하나
    //   timeStep    binding 0 UBO 의 h
    //   gravity     binding 0 UBO 의 gravity (바람을 더한 값이 올라간다)
    // 앞의 둘은 커맨드 버퍼를 기록할 때 읽히므로 GPU 와 경쟁하지 않는다.
    void Cloth::applyLiveParams()
    {
        if (!this->clothData.isReady())
        {
            return;
        }

        this->clothData.setIterations(this->pendingInfo.iterations);
        this->clothData.setFloorY(this->pendingInfo.floorY);

        // 바람을 중력에 접어 넣는다. 셰이더는 바람의 존재를 모른다.
        //   일정한 힘 F  ->  가속도 F/m  ->  gravity 에 더하면 끝
        // pendingInfo.gravity 는 '기본 중력' 으로 남겨 둔다. 그러지 않으면
        // 돌풍이 매 프레임 슬라이더 값을 덮어써서 슬라이더를 못 잡는다.
        cVec3 wind = cVec3(0.0f);

        if (this->windEnabled)
        {
            const cFloat dirLength = glm::length(this->windDirection);

            // 방향 X 와 Z 를 둘 다 0 으로 끌면 길이가 0 이 된다.
            // 그대로 나누면 NaN 이 나오고, 그 NaN 이 UBO -> 예측 위치 ->
            // 정점 버퍼로 퍼져서 천이 통째로 사라진다. 그냥 바람 0 으로 둔다.
            if (dirLength > 1e-6f)
            {
                wind = (this->windDirection / dirLength)
                     * this->windStrength
                     * this->getGustFactor();
            }
        }

        SimulationParams params{};
        params.gravity = this->pendingInfo.gravity + wind;
        params.h = this->pendingInfo.timeStep;

        this->clothData.setSimulationParams(params);
    }

    // 돌풍이 꺼져 있으면 1. 켜져 있으면 1 을 가운데로 sin 으로 흔든다.
    // windGustAmount 가 1 이면 0배 ~ 2배까지 가므로 바람이 완전히 멎는
    // 순간이 생긴다. 천이 한 번 가라앉았다 다시 들린다.
    cFloat Cloth::getGustFactor() const
    {
        if (!this->windGust)
        {
            return 1.0f;
        }

        return 1.0f + this->windGustAmount * std::sin(this->windGustSpeed * this->windTime);
    }

    // 돌풍의 위상만 진행시킨다. 실제 값 반영은 applyLiveParams() 가 한다.
    //
    // 일시정지 중에는 위상도 멈춰야 한다. 그러지 않으면 정지 상태에서 UBO 의
    // gravity 만 계속 바뀌어, 재생을 누른 순간 엉뚱한 바람으로 튄다.
    void Cloth::updateWind(cFloat deltaTime)
    {
        if (!this->windEnabled || !this->windGust || this->simulationPaused)
        {
            return;
        }

        this->windTime += deltaTime;

        this->applyLiveParams();
    }

    void Cloth::readbackCloth()
    {
        this->cxt->waitIdle();
        this->clothData.downloadVertices(this->readbackVertices);

        if (this->readbackVertices.empty())
        {
            this->hasReadback = false;
            return;
        }

        this->readbackMinY = this->readbackVertices[0].position.y;
        this->readbackMaxY = this->readbackVertices[0].position.y;
        this->readbackNanCount = 0;

        cVec3 sum = cVec3(0.0f);

        for (const ClothVertex& vertex : this->readbackVertices)
        {
            const cVec3& position = vertex.position;

            // 발산하면 NaN/Inf 가 먼저 나타난다.
            // 이 값이 0 이 아니면 반복 횟수나 h 가 아니라 어딘가가 틀린 것이다.
            if (std::isnan(position.x) || std::isnan(position.y) || std::isnan(position.z) ||
                std::isinf(position.x) || std::isinf(position.y) || std::isinf(position.z))
            {
                this->readbackNanCount++;
                continue;
            }

            this->readbackMinY = std::min(this->readbackMinY, position.y);
            this->readbackMaxY = std::max(this->readbackMaxY, position.y);
            sum += position;
        }

        this->readbackCenter = sum / static_cast<cFloat>(this->readbackVertices.size());
        this->readbackFrame = this->frameCounter;
        this->hasReadback = true;

        Log::printToLogger("[Cloth] readback frame %u : y %.4f ~ %.4f / center (%.4f, %.4f, %.4f) / nan %u",
                           this->readbackFrame,
                           this->readbackMinY, this->readbackMaxY,
                           this->readbackCenter.x, this->readbackCenter.y, this->readbackCenter.z,
                           this->readbackNanCount);
    }

    // -----------------------------------------------------------------
    // Application::run() 과 같은 내용이고, 천 compute 기록 한 줄만 다르다.
    // RenderGraph 는 Image2D 만 리소스로 받으므로 천 버퍼를 패스로 등록할 수 없다.
    // 그래서 renderer.render() (= renderGraph.execute()) 앞에 직접 기록한다.
    // -----------------------------------------------------------------
    void Cloth::run()
    {
        _VK_CHECK_RESULT_(vkWaitForFences(this->cxt->getDevice()->logicaldevice, 1,
                                          &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
        _VK_CHECK_RESULT_(vkResetFences(this->cxt->getDevice()->logicaldevice, 1,
                                        &this->inFlightFences[currentFrame]));

        renderer.update(*camera.get(), currentFrame, (float)glfwGetTime() * 0.5f);
        renderer.updateBoneData(models, currentFrame);

        cMat4 viewProjection = this->camera->getProjectionMatrix() * this->camera->getViewMatrix();
        renderer.updateViewFrustum(viewProjection);

        for (auto& model : models)
        {
            const cMat4& modelMatrix = model.getModelResource().modelMatrix;
            for (auto& mesh : model.getMeshes())
            {
                mesh.updateWorldBounds(modelMatrix);
            }
        }

        renderer.performFrustumCulling(models);
        guiRenderer.update();

        cUint32_t imageIndex{0};
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

        CommandBufferHandle& cmd = this->commandBuffers[currentFrame];

        vkResetCommandBuffer(cmd.getCommandBuffer(), 0);
        VkCommandBufferBeginInfo cmdBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        _VK_CHECK_RESULT_(vkBeginCommandBuffer(cmd.getCommandBuffer(), &cmdBufferBeginInfo));
        {
            VkViewport viewport{0.0f, 0.0f, (float)this->extent.width, (float)this->extent.height, 0.0f, 1.0f};
            VkRect2D scissor{0, 0, this->extent.width, this->extent.height};

            // ---- 천 시뮬레이션 (Vertex Block Descent) --------------------
            // 그래픽 패스보다 먼저. cloth 패스가 이 결과를 정점 버퍼로 읽는다.
            if (!this->simulationPaused)
            {
                this->clothData.step(cmd.getCommandBuffer());
            }

            this->renderer.render(cmd.getCommandBuffer(), currentFrame, imageIndex,
                                  this->models, viewport, scissor);

            this->guiRenderer.draw(cmd.getCommandBuffer(),
                                   this->swapChain->getSwapChainImageView(imageIndex), viewport);

            this->swapChain->transitionTo(cmd.getCommandBuffer(), imageIndex);
        }
        _VK_CHECK_RESULT_(vkEndCommandBuffer(cmd.getCommandBuffer()));

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        {
            this->framebufferResized = false;
            recreateSwapchain();
        }
        else if (presentResult != VK_SUCCESS)
        {
            Log::exitToLogger("Failed to present swapchain image!");
        }
    }

    void Cloth::update()
    {
        auto lastTime = std::chrono::high_resolution_clock::now();
        cFloat deltaTime = 0.016f;

        while (!window->shouldClose())
        {
            window->getMouseState().mouseDelta = cVec3(0.0f);
            window->pollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            deltaTime = std::min(deltaTime, 0.033f);

            this->updateGui();

            this->updateCameraLook();
            this->updateCameraMove(deltaTime);
            camera->update();

            this->renderer.getSceneDataUBO().projection = this->camera->getProjectionMatrix();
            this->renderer.getSceneDataUBO().view = this->camera->getViewMatrix();
            this->renderer.getSceneDataUBO().cameraPos = camera->getPos();

            for (auto& model : models)
            {
                if (model.hasAnimations())
                {
                    model.updateAnimation(deltaTime);
                }
            }

            this->updateLightSpaceMatrix();

            // 돌풍이 켜져 있으면 UBO 의 gravity 가 매 프레임 갱신된다.
            // run() 보다 먼저 해야 이번 프레임의 compute 에 반영된다.
            this->updateWind(deltaTime);

            // 천 compute 는 run() 안에서 프레임 커맨드 버퍼에 기록한다.
            this->run();

            this->currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
            this->currentSemaphore = (this->currentSemaphore + 1) % this->swapChain->getSwapChainImageCount();

            this->frameCounter++;

            if (!this->autoReadbackDone && this->frameCounter >= kAutoReadbackFrame)
            {
                this->readbackCloth();
                this->autoReadbackDone = true;
            }
        }

        this->cxt->waitIdle();
    }

    void Cloth::updateCameraLook()
    {
        auto& mouse = window->getMouseState();

        // mouseDelta 는 오른쪽 버튼 드래그 중에만 채워진다 (Window.cpp 참고).
        if (mouse.mouseDelta.x == 0.0f && mouse.mouseDelta.y == 0.0f)
        {
            return;
        }

        camera->rotateDeltaRotation(
            cVec3(mouse.mouseDelta.x * LOOK_SENSITIVITY * INVERT_X,
                  mouse.mouseDelta.y * LOOK_SENSITIVITY * INVERT_Y,
                  0.0f));
    }

    void Cloth::updateCameraMove(cFloat deltaTime)
    {
        const auto& keys = window->getKeyboardState();

        cFloat speed = CAMERA_BASE_SPEED;
        if (keys.m_keyPressed[GLFW_KEY_LEFT_SHIFT])
        {
            speed *= CAMERA_BOOST_MUL;
        }
        if (keys.m_keyPressed[GLFW_KEY_LEFT_ALT])
        {
            speed *= CAMERA_SLOW_MUL;
        }
        camera->setSpeed(speed);

        if (keys.m_keyPressed[GLFW_KEY_W])
        {
            camera->moveForward(deltaTime);
        }
        if (keys.m_keyPressed[GLFW_KEY_S])
        {
            camera->moveForward(-deltaTime);
        }
        if (keys.m_keyPressed[GLFW_KEY_D])
        {
            camera->moveRight(deltaTime);
        }
        if (keys.m_keyPressed[GLFW_KEY_A])
        {
            camera->moveRight(-deltaTime);
        }
        if (keys.m_keyPressed[GLFW_KEY_E])
        {
            camera->moveUp(deltaTime);
        }
        if (keys.m_keyPressed[GLFW_KEY_Q])
        {
            camera->moveUp(-deltaTime);
        }
    }

    void Cloth::updateLightSpaceMatrix()
    {
        if (models.empty())
        {
            return;
        }

        cMat4 lightView = glm::lookAt(cVec3(0.0f),
                                      -renderer.getSceneDataUBO().directionalLightDir,
                                      cVec3(0.0f, 0.0f, 1.0f));

        cVec3 firstMin = cVec3(models[0].getModelResource().modelMatrix * cVec4(models[0].getBoundingBoxMin(), 1.0f));
        cVec3 firstMax = cVec3(models[0].getModelResource().modelMatrix * cVec4(models[0].getBoundingBoxMax(), 1.0f));

        cVec3 min_ = glm::min(firstMin, firstMax);
        cVec3 max_ = glm::max(firstMin, firstMax);

        for (cUint32_t i = 1; i < static_cast<cUint32_t>(models.size()); i++)
        {
            cVec3 modelMin = cVec3(models[i].getModelResource().modelMatrix * cVec4(models[i].getBoundingBoxMin(), 1.0f));
            cVec3 modelMax = cVec3(models[i].getModelResource().modelMatrix * cVec4(models[i].getBoundingBoxMax(), 1.0f));

            min_ = glm::min(min_, glm::min(modelMin, modelMax));
            max_ = glm::max(max_, glm::max(modelMin, modelMax));
        }

        // 천도 그림자 범위에 들어가야 한다 (천은 아직 그림자 패스에 들어가지 않는다).
        const ClothSimulation::Info& clothInfo = this->cloth.getInfo();
        const cVec3 clothMin = clothInfo.origin
            + cVec3(0.0f, -clothInfo.spacing * static_cast<cFloat>(clothInfo.gridY - 1u), -clothInfo.spacing);
        const cVec3 clothMax = clothInfo.origin
            + cVec3(clothInfo.spacing * static_cast<cFloat>(clothInfo.gridX - 1u), 0.0f, clothInfo.spacing);

        min_ = glm::min(min_, glm::min(clothMin, clothMax));
        max_ = glm::max(max_, glm::max(clothMin, clothMax));

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

        for (cSize i = 0; i != 8; i++)
        {
            auto temp = cVec3(lightView * cVec4(corners[i], 1.0f));
            vmin = glm::min(vmin, temp);
            vmax = glm::max(vmax, temp);
        }

        min_ = vmin;
        max_ = vmax;

        cMat4 lightProjection = glm::orthoLH_ZO(min_.x, max_.x, min_.y, max_.y, max_.z, min_.z); // 마지막 Max, Min 순서 주의
        renderer.getSceneDataUBO().lightSpaceMatrix = lightProjection * lightView;
    }

    // -----------------------------------------------------------------
    // GUI
    //
    // Application::updateGui() 는 마지막에 ImGui::Render() 를 호출하므로
    // 그것을 그대로 부르고 창을 더 띄울 수 없다. 같은 순서를 여기서 다시 구성한다.
    // -----------------------------------------------------------------
    void Cloth::updateGui()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(float(extent.width), float(extent.height));

        platform::MouseState& mouseState = this->window->getMouseState();
        io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
        io.MouseDown[0] = mouseState.buttons.left;
        io.MouseDown[1] = mouseState.buttons.right;
        io.MouseDown[2] = mouseState.buttons.middle;

        // 휠을 ImGui 로 넘긴다. 넘기지 않으면 내용이 창보다 긴 패널을
        // 스크롤바를 직접 끄는 것 말고는 움직일 수 없다.
        // 카메라 줌은 handleScroll() 이 따로 하므로 그대로 같이 동작한다.
        io.MouseWheel = mouseState.scrollDelta;
        mouseState.scrollDelta = 0.0f;

        ImGui::NewFrame();

        this->renderClothControlGUI();

        this->renderCameraControlWindow();
        this->renderHDRControlWindow();
        this->renderPostProcessingControlWindow();
        this->renderSSAOControlWindow();

        ImGui::Render();
    }

    void Cloth::renderClothControlGUI()
    {
        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(460.0f, 0.0f), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("천 시뮬레이션 (Vertex Block Descent)", nullptr, ImGuiWindowFlags_None))
        {
            ImGui::End();
            return;
        }

        const ClothSimulation::Info& active = this->cloth.getInfo();
        const cUint32_t groupCount = this->cloth.getGroupCount();

        ImGui::Text("격자      : %u x %u", active.gridX, active.gridY);
        ImGui::Text("정점      : %u", this->cloth.getVertexCount());
        ImGui::Text("스프링    : %u", this->cloth.getSpringCount());
        ImGui::Text("인덱스    : %u", this->cloth.getIndexCount());
        ImGui::Text("그룹      : %u", groupCount);
        ImGui::Text("dispatch / 프레임 : %u   (1 + %u x %u + 1)",
                    this->clothData.getDispatchCount(), groupCount, active.iterations);
        ImGui::Text("배리어   / 프레임 : %u", this->clothData.getBarrierCount());

        ImGui::Separator();
        ImGui::Text("검증");
        ImGui::Text("  인접 정보 : %s", this->cloth.isAdjacencyValid() ? "OK" : "FAILED");
        ImGui::Text("  그룹 분할 : %s", this->cloth.isGroupingValid() ? "OK" : "FAILED");
        ImGui::Text("  GPU 데이터 : %s", this->clothData.isReady() ? "준비됨" : "준비 안 됨");

        ImGui::Separator();
        ImGui::Text("시뮬레이션");
        ImGui::Checkbox("일시정지", &this->simulationPaused);

        // 눈으로 보는 것과 별개로 숫자로도 확인한다.
        // 중력만 받는 천이라면 min y 가 계속 내려가다가 멈춰야 하고,
        // nan 이 하나라도 생기면 발산한 것이다.
        if (ImGui::Button("정점 읽어오기"))
        {
            this->readbackCloth();
        }

        if (this->hasReadback)
        {
            ImGui::Text("  프레임   : %u", this->readbackFrame);
            ImGui::Text("  y 범위   : %.4f ~ %.4f", this->readbackMinY, this->readbackMaxY);
            ImGui::Text("  중심     : %.4f, %.4f, %.4f",
                        this->readbackCenter.x, this->readbackCenter.y, this->readbackCenter.z);
            ImGui::Text("  nan/inf  : %u %s",
                        this->readbackNanCount,
                        this->readbackNanCount == 0u ? "(OK)" : "(DIVERGED)");
        }
        else
        {
            ImGui::Text("  (아직 읽지 않았다)");
        }

        if (ImGui::CollapsingHeader("그룹 범위"))
        {
            const std::vector<ClothSimulation::GroupRange>& ranges = this->cloth.getGroupRanges();

            for (cUint32_t g = 0; g < static_cast<cUint32_t>(ranges.size()); g++)
            {
                ImGui::Text("  그룹 %u : offset %u, count %u", g, ranges[g].offset, ranges[g].count);
            }
        }

        // ---- 재빌드가 필요 없는 값들 ----------------------------------
        // 버퍼 내용을 바꾸지 않으므로 Apply 도 waitIdle() 도 필요 없다.
        // 슬라이더를 드래그하는 동안 프레임이 끊기지 않는다.
        ImGui::Separator();
        ImGui::Text("실시간 값 (재빌드 없이 바로 적용된다)");

        cInt liveIterations = static_cast<cInt>(this->pendingInfo.iterations);

        // 품질 손잡이. 줄이면 처짐이 남아 천이 물렁해 보인다.
        // 비용은 dispatch / frame = 2 + groups x iterations 로 직접 곱해진다.
        if (ImGui::SliderInt("반복 횟수", &liveIterations, 1, 50))
        {
            this->pendingInfo.iterations = static_cast<cUint32_t>(liveIterations);
            this->applyLiveParams();
        }

        // h 를 줄이면 관성 강성 m/h^2 가 커져 천이 뻣뻣해지고 수치 감쇠도 줄어든다.
        // 프레임이 느려지는 게 아니라 한 프레임이 더 짧은 시간이 된다.
        if (ImGui::SliderFloat("시간 간격 h", &this->pendingInfo.timeStep,
                               1.0f / 240.0f, 1.0f / 30.0f, "%.5f"))
        {
            this->applyLiveParams();
        }

        if (ImGui::SliderFloat("중력 Y", &this->pendingInfo.gravity.y, -30.0f, 10.0f))
        {
            this->applyLiveParams();
        }

        // 임시 바닥. 에너지 항이 아니라 clothFinalize.comp 의 위치 클램프다.
        if (ImGui::SliderFloat("바닥 Y", &this->pendingInfo.floorY, -5.0f, 5.0f))
        {
            this->applyLiveParams();
        }

        // restLength 를 생성 당시의 실제 거리로 잡았으므로, 중력과 바람이
        // 모두 0 이면 천은 전혀 움직이지 않아야 한다.
        // 데이터가 맞는지 보는 가장 빠른 검사다.
        if (ImGui::Button("중력 켜고 끄기"))
        {
            this->pendingInfo.gravity.y = (this->pendingInfo.gravity.y == 0.0f) ? -9.8f : 0.0f;
            this->applyLiveParams();
        }
        ImGui::SameLine();
        ImGui::Text("(중력 0 + 바람 끄면 천이 전혀 움직이지 않아야 한다)");

        // ---- 바람 ------------------------------------------------------
        // 셰이더 수정 0 줄이다. 바람은 가속도로 바뀌어 gravity 에 합쳐진다.
        // jitter Z 를 0 으로 두고 바람만 켜도 주름이 생긴다.
        ImGui::Separator();
        ImGui::Text("바람 (가속도로 바꿔 중력에 더한다)");

        cBool windChanged = false;

        // 켜고 끄기. 끄면 아래 값들은 그대로 남고 바람만 0 이 된다.
        windChanged = ImGui::Checkbox("바람 사용", &this->windEnabled) || windChanged;

        // 꺼져 있을 때 회색으로 만들어 둔다. 값은 유지되므로 다시 켜면 그대로다.
        ImGui::BeginDisabled(!this->windEnabled);

        // 가속도라 중력(9.8)과 같은 단위로 비교하면 된다.
        // 2.0 = 중력의 1/5. 20 이면 천이 거의 수평으로 날아간다.
        windChanged = ImGui::SliderFloat("세기", &this->windStrength, 0.0f, 20.0f, "%.2f") || windChanged;

        // 길이는 의미가 없다. 정규화해서 쓰므로 X 와 Z 의 비율만 보면 된다.
        windChanged = ImGui::SliderFloat("방향 X", &this->windDirection.x, -1.0f, 1.0f, "%.2f") || windChanged;
        windChanged = ImGui::SliderFloat("방향 Z", &this->windDirection.z, -1.0f, 1.0f, "%.2f") || windChanged;

        // 돌풍. 깃발이 펄럭이는 움직임이 이것만으로 나온다.
        windChanged = ImGui::Checkbox("돌풍", &this->windGust) || windChanged;

        if (this->windGust)
        {
            windChanged = ImGui::SliderFloat("돌풍 속도", &this->windGustSpeed, 0.1f, 5.0f) || windChanged;
            windChanged = ImGui::SliderFloat("돌풍 흔들림", &this->windGustAmount, 0.0f, 1.0f) || windChanged;
        }

        ImGui::EndDisabled();

        if (ImGui::Button("바람 기본값"))
        {
            this->windStrength = 2.0f;
            this->windDirection = cVec3(0.0f, 0.0f, 1.0f);
            this->windGust = false;
            this->windTime = 0.0f;
            windChanged = true;
        }
        ImGui::SameLine();

        // 돌풍까지 반영한, 지금 이 프레임에 실제로 들어가는 세기.
        ImGui::Text("실효 세기 %.2f   돌풍 위상 %.1f s",
                    this->windEnabled ? this->windStrength * this->getGustFactor() : 0.0f,
                    this->windTime);

        if (windChanged)
        {
            this->applyLiveParams();
        }

        // ---- 재빌드가 필요한 값들 --------------------------------------
        // 정점/스프링/그룹 구성이나 버퍼 내용을 바꾸므로 Apply 를 눌러야 한다.
        ImGui::Separator();
        ImGui::Text("재빌드가 필요한 값 (Apply 를 눌러야 적용된다)");

        cInt gridX = static_cast<cInt>(this->pendingInfo.gridX);
        cInt gridY = static_cast<cInt>(this->pendingInfo.gridY);

        // 그룹 분할(DSATUR)이 정점 수에 민감하므로 상한을 64 로 둔다.
        if (ImGui::SliderInt("격자 X", &gridX, 2, 64))
        {
            this->pendingInfo.gridX = static_cast<cUint32_t>(gridX);
            this->clothDirty = true;
        }
        if (ImGui::SliderInt("격자 Y", &gridY, 2, 64))
        {
            this->pendingInfo.gridY = static_cast<cUint32_t>(gridY);
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("정점 간격", &this->pendingInfo.spacing, 0.01f, 0.20f))
        {
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("정점 질량", &this->pendingInfo.mass, 0.001f, 0.1f, "%.4f"))
        {
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("k 늘어남", &this->pendingInfo.kStretch, 100.0f, 20000.0f))
        {
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("k 전단", &this->pendingInfo.kShear, 0.0f, 10000.0f))
        {
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("k 굽힘", &this->pendingInfo.kBend, 0.0f, 1000.0f))
        {
            this->clothDirty = true;
        }
        if (ImGui::Checkbox("전단 스프링 사용", &this->pendingInfo.useShear))
        {
            this->clothDirty = true;
        }
        if (ImGui::Checkbox("굽힘 스프링 사용", &this->pendingInfo.useBend))
        {
            this->clothDirty = true;
        }
        if (ImGui::Checkbox("위쪽 두 모서리 고정", &this->pendingInfo.pinTopCorners))
        {
            this->clothDirty = true;
        }
        if (ImGui::SliderFloat("초기 Z 흔들림", &this->pendingInfo.initialJitterZ, 0.0f, 0.005f, "%.5f"))
        {
            this->clothDirty = true;
        }

        ImGui::Separator();

        if (this->clothDirty)
        {
            ImGui::Text("바뀐 값이 있다 - 적용을 눌러야 반영된다");
        }

        if (ImGui::Button("적용 (재빌드)"))
        {
            this->rebuildCloth(this->pendingInfo);
        }

        ImGui::SameLine();

        if (ImGui::Button("되돌리기"))
        {
            this->pendingInfo = this->cloth.getInfo();
            this->clothDirty = false;

            // 실시간 값도 pendingInfo 를 따라가야 한다.
            // 빼먹으면 슬라이더는 되돌아갔는데 clothData 는 그대로 남는다.
            this->applyLiveParams();
        }

        ImGui::SameLine();

        // 같은 설정으로 데이터를 다시 올린다 = 처음 상태로 되돌리기
        if (ImGui::Button("기본값으로"))
        {
            this->rebuildCloth(this->cloth.getInfo());
        }

        ImGui::Separator();
        ImGui::Text("남은 일 : ImGui 로 천 머티리얼 조절, 정점 법선,");
        ImGui::Text("          그림자 패스에 천 포함, 접촉을 에너지 항으로");

        ImGui::End();
    }
}
