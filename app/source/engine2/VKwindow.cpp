#include "VKwindow.h"

namespace vkengine {
    namespace platform {
        void BaseWindow::setWindowResizeCallback(WindowResizeCallback callback)
        {
            this->m_resizeCallback = std::move(callback);
        }

        void BaseWindow::setWindowCloseCallback(WindowCloseCallback callback)
        {
            this->m_closeCallback = std::move(callback);
        }

        void BaseWindow::setKeyCallback(KeyCallback callback)
        {
            m_keyCallback = std::move(callback);
        }

        void BaseWindow::setMouseButtonCallback(MouseButtonCallback callback)
        {
            m_mouseButtonCallback = std::move(callback);
        }

        void BaseWindow::setMouseMoveCallback(MouseMoveCallback callback)
        {
            m_mouseMoveCallback = std::move(callback);
        }

        void BaseWindow::setScrollCallback(ScrollCallback callback)
        {
            m_scrollCallback = std::move(callback);
        }

        void BaseWindow::setWindowFocusCallback(WindowFocusCallback callback)
        {
            m_focusCallback = std::move(callback);
        }
        
        std::unique_ptr<VirtualWindows> WindowFactory::create(Backend backend) {
            switch (backend) {
            case Backend::GLFW:
                return std::make_unique<glfw::glfwWindow>();
            default:
                return nullptr;
            }
        }

        // WindowManager 구현
        WindowManager* WindowManager::s_instance = nullptr;

        WindowManager& WindowManager::getInstance() {
            if (!s_instance) {
                s_instance = new WindowManager();
            }
            return *s_instance;
        }

        void WindowManager::registerWindow(std::shared_ptr<VirtualWindows> window) {
            m_windows.push_back(std::weak_ptr<VirtualWindows>(window));
        }

        void WindowManager::pollAllEvents() {
            // 모든 등록된 윈도우의 이벤트 폴링
            for (auto it = m_windows.begin(); it != m_windows.end();) {
                if (auto window = it->lock()) {
                    window->pollEvents();
                    ++it;
                }
                else {
                    // weak_ptr이 만료된 경우 제거
                    it = m_windows.erase(it);
                }
            }
        }

        void WindowManager::cleanup() {
            m_windows.clear();
        }

        namespace glfw {
            glfwWindow::glfwWindow()
            {
                if (!glfwInit()) {
                    _EXIT_WITH_MESSAGE_("Failed to initialize GLFW\n");
                    return;
                }

                m_isCreated = false; // 초기화 상태 설정
            }

            glfwWindow::~glfwWindow()
            {
                if (m_window) {
                    glfwDestroyWindow(m_window);
                }
                glfwTerminate();
            }

            bool glfwWindow::createWindow(const WindowCreateInfo& createInfo)
            {
                m_createInfo = createInfo;

                // GLFW 윈도우 힌트 설정
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                glfwWindowHint(GLFW_RESIZABLE, m_createInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_VISIBLE, m_createInfo.visible ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_DECORATED, m_createInfo.decorated ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_FOCUSED, m_createInfo.focused ? GLFW_TRUE : GLFW_FALSE);
                glfwWindowHint(GLFW_MAXIMIZED, m_createInfo.maximized ? GLFW_TRUE : GLFW_FALSE);

                m_window = glfwCreateWindow(
                    m_createInfo.width,
                    m_createInfo.height,
                    m_createInfo.title.c_str(),
                    nullptr,
                    nullptr
                );

                if (!m_window) {
                    _EXIT_WITH_MESSAGE_("Failed to create GLFW window\n");
                    return false;
                }

                // 사용자 포인터 설정 (콜백에서 this 포인터 접근용)
                glfwSetWindowUserPointer(m_window, this);

                // 콜백 함수들 등록
                glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
                glfwSetWindowCloseCallback(m_window, windowCloseCallback);
                glfwSetKeyCallback(m_window, keyCallback);
                glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
                glfwSetCursorPosCallback(m_window, cursorPosCallback);
                glfwSetScrollCallback(m_window, scrollCallback);
                glfwSetWindowFocusCallback(m_window, windowFocusCallback);

                m_isCreated = true;
                return true;
            }


            void glfwWindow::pollEvents()
            {
            }

            void glfwWindow::waitEvents()
            {
            }

            cBool glfwWindow::shouldClose()
            {
                return cBool();
            }

            void glfwWindow::setShouldClose(cBool value)
            {
            }

            void glfwWindow::getFramebufferSize(int* width, int* height)
            {
                if (m_window) {
                    glfwGetFramebufferSize(m_window, width, height);
                }
                else {
                    *width = 0;
                    *height = 0;
                }
            }

            VkSurfaceKHR glfwWindow::createVulkanSurface(VkInstance instance)
            {
                VkSurfaceKHR surface;

                _VK_CHECK_RESULT_(glfwCreateWindowSurface(instance, this->m_window, nullptr, &surface));

                return surface;
            }

            void glfwWindow::framebufferSizeCallback(GLFWwindow* window, cInt width, cInt height)
            {
                glfwWindow* instance = static_cast<glfwWindow*>(glfwGetWindowUserPointer(window));
                if (instance && instance->m_resizeCallback) {
                    instance->m_resizeCallback(width, height);
                }
            }

            void glfwWindow::windowCloseCallback(GLFWwindow* window)
            {

            }

            void glfwWindow::keyCallback(GLFWwindow* window, cInt key, cInt scancode, cInt action, cInt mods)
            {

            }

            void glfwWindow::mouseButtonCallback(GLFWwindow* window, cInt button, cInt action, cInt mods)
            {

            }

            void glfwWindow::cursorPosCallback(GLFWwindow* window, cDouble xpos, cDouble ypos)
            {

            }

            void glfwWindow::scrollCallback(GLFWwindow* window, cDouble xoffset, cDouble yoffset)
            {

            }

            void glfwWindow::windowFocusCallback(GLFWwindow* window, cInt focused)
            {

            }

        }
    }
}