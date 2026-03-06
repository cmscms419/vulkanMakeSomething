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
        //   shaderManager{*this->cxt,
        //                 RootPath + SHADER_PATH,
        //                 {
        //                     {"shadowMap", {"vertshadowMap.spv", "fragshadowMap.spv"}},
        //                     {"pbrForward", {"vertpbrForward.spv", "fragpbrForward.spv"}},
        //                     {"gui", {"vertimgui.spv", "fragimgui.spv"}},
        //                     {"sky", {"vertskybox2.spv", "fragskybox2.spv"}},
        //                     {"post", {"vertpost.spv", "fragpost.spv"}},
        //                     {"ssao", {"compssao.spv"}},
        //                 }},
          guiRenderer{*this->cxt, shaderManager, swapChain->getSwapChainImageFormat(), RootPath + this->AssetsPath},
          forwardRenderer(*this->cxt, shaderManager, this->kMaxFramesInFlight, RootPath + this->AssetsPath, RootPath + this->ShaderPath)
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

    }

    void Application3::loadModels(const std::vector<ModelConfig> &modelConfigs)
    {
        
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
        this->forwardRenderer.buildRenderGraph(*this->swapChain);
    }
    void Application3::renderHDRControlWindow()
    {
    }
    void Application3::renderPostProcessingControlWindow()
    {
    }
    void Application3::renderCameraControlWindow()
    {
    }
    void Application3::run()
    {
    }

    void vkengine::Application3::update()
    {
    }

    void vkengine::Application3::updateGui()
    {
    }
}
