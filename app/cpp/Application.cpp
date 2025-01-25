
#include "../../include/source/Application.h"

using namespace vkutil;

namespace vkutil {

    void Application::init() {
        initWindow();
        initVulkan();
    }

    bool Application::run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // GLFW �̺�Ʈ ť ó��
            update(); // ������Ʈ
#ifdef DEBUG_
            printf("update\n");
#endif // DEBUG_

        }

        state = false;

        return state;
    }

    void Application::update() {
        // ������� ������Ʈ
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
        glfwSetKeyCallback(this->window, vkutil::key_callback);  // Ű �Է� �ݹ� ����
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
        // VkApplicationInfo ����ü�� ���ø����̼ǿ� ���� ������ Vulkan���� �����ϱ� ���� ���˴ϴ�.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // ����ü Ÿ���� �����մϴ�.
        appInfo.pApplicationName = "Vulkan Test";               // ���ø����̼� �̸��� �����մϴ�.
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // ���ø����̼� ������ �����մϴ�.
        appInfo.pEngineName = "cms491 Engine";                      // ���� �̸��� �����մϴ�.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // ���� ������ �����մϴ�.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // ����� Vulkan API ������ �����մϴ�.

        // VkInstanceCreateInfo ����ü�� Vulkan �ν��Ͻ��� �����ϱ� ���� ������ �����մϴ�.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // ����ü Ÿ���� �����մϴ�.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo ����ü�� �����մϴ�.

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;                                             // Ȯ�� ����� ������ ������ �����մϴ�.
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // �ʿ��� GLFW Ȯ�� ����� �����ɴϴ�.
        createInfo.enabledExtensionCount = glfwExtensionCount;                   // Ȱ��ȭ�� Ȯ�� ������ �����մϴ�.
        createInfo.ppEnabledExtensionNames = glfwExtensions;                     // Ȱ��ȭ�� Ȯ�� ����� �����մϴ�.
        createInfo.enabledLayerCount = 0;                                        // Ȱ��ȭ�� ���̾� ������ �����մϴ�.

        // Vulkan �ν��Ͻ��� �����մϴ�.
        if (vkCreateInstance(&createInfo, nullptr, &VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // �ν��Ͻ� ���� ���� �� ���ܸ� �߻���ŵ�ϴ�.
        }
        else
        {
            printf("create instance\n");
        }
    }
    
    // vkutil namespace�� key_callback �Լ� ����
    void vkutil::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
            printf("test\n");
        }
    }
}
