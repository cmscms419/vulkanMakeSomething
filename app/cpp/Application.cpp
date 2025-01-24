
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
    }
    
    void Application::mainLoop()
    {

    }
    
    // vkutil namespace에 key_callback 함수 정의
    void vkutil::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
            printf("test\n");
        }
    }
}
