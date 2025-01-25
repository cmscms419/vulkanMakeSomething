
#include "../../include/source/Application.h"

using namespace vkutil;

namespace vkutil {

    void Application::init() {
        initWindow();
        initVulkan();
    }

    bool Application::run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // GLFW 이벤트 큐 처리
            update(); // 업데이트
#ifdef DEBUG_
            printf("update\n");
#endif // DEBUG_

        }

        state = false;

        return state;
    }

    void Application::update() {
        // 만든것을 업데이트
    }

    void Application::setup()
    {

    }

    void Application::cleanup() {
        glfwDestroyWindow(window);
        glfwTerminate();
        vkDestroyInstance(this->VKinstance, nullptr);

#ifdef DEBUG_
        printf("cleanup\n");
#endif // DEBUG_

    }

    void Application::initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->window = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan Test", nullptr, nullptr);

        glfwSetWindowUserPointer(this->window, this);
        glfwSetKeyCallback(this->window, vkutil::key_callback);  // 키 입력 콜백 설정
    }

    void Application::initVulkan() {
        if (glfwVulkanSupported() == GLFW_FALSE) {
            throw std::runtime_error("Vulkan is not supported");
        }

        this->createInstance();
    }
    
    void Application::mainLoop()
    {

    }

    void Application::createInstance()
    {
        // VkApplicationInfo 구조체는 애플리케이션에 대한 정보를 Vulkan에게 제공하기 위해 사용됩니다.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // 구조체 타입을 지정합니다.
        appInfo.pApplicationName = "Vulkan Test";               // 애플리케이션 이름을 지정합니다.
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // 애플리케이션 버전을 지정합니다.
        appInfo.pEngineName = "cms491 Engine";                      // 엔진 이름을 지정합니다.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // 엔진 버전을 지정합니다.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // 사용할 Vulkan API 버전을 지정합니다.

        // VkInstanceCreateInfo 구조체는 Vulkan 인스턴스를 생성하기 위한 정보를 제공합니다.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // 구조체 타입을 지정합니다.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo 구조체를 참조합니다.

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;                                             // 확장 목록을 저장할 변수를 선언합니다.
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // 필요한 GLFW 확장 목록을 가져옵니다.
        createInfo.enabledExtensionCount = glfwExtensionCount;                   // 활성화할 확장 개수를 지정합니다.
        createInfo.ppEnabledExtensionNames = glfwExtensions;                     // 활성화할 확장 목록을 지정합니다.
        createInfo.enabledLayerCount = 0;                                        // 활성화할 레이어 개수를 지정합니다.

        // Vulkan 인스턴스를 생성합니다.
        if (vkCreateInstance(&createInfo, nullptr, &VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // 인스턴스 생성 실패 시 예외를 발생시킵니다.
        }
        else
        {
            printf("create instance\n");
        }
    }
    
    // vkutil namespace에 key_callback 함수 정의
    void vkutil::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
            printf("test\n");
        }
    }
}
