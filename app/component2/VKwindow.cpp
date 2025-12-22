#include "VKwindow.h"

using namespace vkengine::Log;

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

        std::unique_ptr<VirtualWindows> WindowFactory::create(Backend backend, WindowCreateInfo info) {
            std::unique_ptr<VirtualWindows> window;

            switch (backend) {
            case Backend::GLFW:
                window = std::make_unique<glfw::glfwWindow>(info);
                break;
            default:
                window = nullptr;
                break;
            }

            return window;
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
                    EXIT_TO_LOGGER("Failed to initialize GLFW\n");
                    return;
                }

                m_isCreated = false; // 초기화 상태 설정

                this->createWindow(WindowCreateInfo());
            }

            glfwWindow::glfwWindow(const WindowCreateInfo& createInfo)
            {
            
                if (!glfwInit()) {
                    EXIT_TO_LOGGER("Failed to initialize GLFW\n");
                    return;
                }

                m_isCreated = false; // 초기화 상태 설정

                this->createWindow(createInfo);
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
                    EXIT_TO_LOGGER("Failed to create GLFW window\n");
                }
                else {
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
                }

                return m_isCreated;
            }


            void glfwWindow::pollEvents()
            {
                glfwPollEvents();
            }

            void glfwWindow::waitEvents()
            {
            }

            cBool glfwWindow::shouldClose()
            {
                return glfwWindowShouldClose(this->m_window);
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

            std::vector<const char*> glfwWindow::getRequiredExtensions()
            {
                // OS에 따라 어떤 EXT가 필요한지 GLFW를 통해서 가져옵니다.

                std::vector<const char*> instanceExtensions{};

                uint32_t glfwExtensionCount = 0;
                const char** extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

                for (uint32_t i = 0; i < glfwExtensionCount; i++) {
                    if (std::find(instanceExtensions.begin(), instanceExtensions.end(), extensions[i]) ==
                        instanceExtensions.end()) {
                        instanceExtensions.push_back(extensions[i]);
                    }
                }

                instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

                PRINT_TO_LOGGER("GLFW Required Instance Extensions:\n");
                PRINT_TO_LOGGER("GlfwRequiredInstanceExtensions : \n", glfwExtensionCount);
                for (const char* ext : instanceExtensions) {
                    PRINT_TO_LOGGER("\t%s\n", ext);
                }

                return instanceExtensions;
            }

            MouseState& glfwWindow::getmouseState()
            {
                return this->mouseState;
            }

            KeyboardState& glfwWindow::getKeyBoardState()
            {
                return this->keyState;
            }

            object::Camera2* glfwWindow::getCamera()
            {
                if (this->camera != nullptr)
                    return this->camera.get();
                else
                    return nullptr;
            }

            void glfwWindow::setCamera(std::shared_ptr<object::Camera2> camera)
            {
                this->camera = camera;
            }

            GLFWwindow* glfwWindow::getGLFWwindow()
            {
                return this->m_window;
            }

            void glfwWindow::framebufferSizeCallback(GLFWwindow* window, cInt width, cInt height)
            {
                glfwWindow* instance = reinterpret_cast<glfwWindow*>(glfwGetWindowUserPointer(window));
                if (instance && instance->m_resizeCallback) {
                    instance->m_resizeCallback(width, height);
                }
            }

            void glfwWindow::windowCloseCallback(GLFWwindow* window)
            {

            }

            void glfwWindow::keyCallback(GLFWwindow* window, cInt key, cInt scancode, cInt action, cInt mods)
            {

                // Retrieve the glfwWindow instance associated with this GLFWwindow
                glfwWindow* instance = reinterpret_cast<glfwWindow*>(glfwGetWindowUserPointer(window));
                if (!instance) {
                    return; // If no instance is associated, exit early
                }

                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }

                // key 값이 m_keyPressed 배열의 유효 인덱스 범위(0~255)인지 확인 후 접근
                if (key >= 0 && key < static_cast<cInt>(sizeof(instance->getKeyBoardState().m_keyPressed) / sizeof(instance->getKeyBoardState().m_keyPressed[0])))
                {
                    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                        instance->keyState.m_keyPressed[key] = true;
                    }
                    else if (action == GLFW_RELEASE) {
                        instance->keyState.m_keyPressed[key] = false;
                    }
                }
            }

            void glfwWindow::mouseButtonCallback(GLFWwindow* window, cInt button, cInt action, cInt mods)
            {

                // Retrieve the glfwWindow instance associated with this GLFWwindow
                glfwWindow* instance = reinterpret_cast<glfwWindow*>(glfwGetWindowUserPointer(window));
                if (instance != nullptr)
                {
                    if (instance->mouseState.clicked)
                    {
                        if (action == GLFW_PRESS) {
                            switch (button) {
                            case GLFW_MOUSE_BUTTON_LEFT:
                                instance->mouseState.buttons.left = true;
                                break;
                            case GLFW_MOUSE_BUTTON_RIGHT:
                                instance->mouseState.buttons.right = true;
                                break;
                            case GLFW_MOUSE_BUTTON_MIDDLE:
                                instance->mouseState.buttons.middle = true;
                                break;
                            }
                        }
                        else if (action == GLFW_RELEASE) {
                            switch (button) {
                            case GLFW_MOUSE_BUTTON_LEFT:
                                instance->mouseState.buttons.left = false;
                                break;
                            case GLFW_MOUSE_BUTTON_RIGHT:
                                instance->mouseState.buttons.right = false;
                                instance->mouseState.move = false;
                                break;
                            case GLFW_MOUSE_BUTTON_MIDDLE:
                                instance->mouseState.buttons.middle = false;
                                break;
                            }
                        }
                    }
                }
            }

            void glfwWindow::cursorPosCallback(GLFWwindow* window, cDouble xpos, cDouble ypos)
            {
                // GLFW 윈도우 상에서의 마우스 x,y 좌표를 가져온다
                glfwGetCursorPos(window, &xpos, &ypos);

                // Retrieve the glfwWindow instance associated with this GLFWwindow
                glfwWindow* instance = reinterpret_cast<glfwWindow*>(glfwGetWindowUserPointer(window));

                if (instance != nullptr)
                {
                    if (instance->mouseState.CameraMove)
                    {
                        if (instance->mouseState.buttons.right)
                        {
                            if (!instance->mouseState.move)
                            {
                                instance->mouseState.position.x = static_cast<cFloat>(xpos);
                                instance->mouseState.position.y = static_cast<cFloat>(ypos);
                                instance->mouseState.move = true;
                            }
                            else
                            {
                                cFloat xoffset = instance->mouseState.position.x - static_cast<cFloat>(xpos);
                                cFloat yoffset = instance->mouseState.position.y - static_cast<cFloat>(ypos);

                                cVec3 force = cVec3(xoffset, yoffset, 0.0f);

                                if (object::Camera2* cam = instance->getCamera())
                                {
                                    cam->RotateDeltaRotation(force);
                                }

                                instance->mouseState.position.x = static_cast<cFloat>(xpos);
                                instance->mouseState.position.y = static_cast<cFloat>(ypos);
                            }
                        }
                        else
                        {
                            instance->mouseState.position.x = static_cast<cFloat>(xpos);
                            instance->mouseState.position.y = static_cast<cFloat>(ypos);
                        }
                    }
                }

                return;
            }

            void glfwWindow::scrollCallback(GLFWwindow* window, cDouble xoffset, cDouble yoffset)
            {
                glfwWindow* instance = reinterpret_cast<glfwWindow*>(glfwGetWindowUserPointer(window));

                if (instance != nullptr)
                {
                    if (instance->mouseState.scrolled)
                    {
                        if (yoffset > 0) {
                            cFloat fov = instance->getCamera()->getFov() - 1.0f;
                            instance->getCamera()->setFov(fov);
                        }
                        else if (yoffset < 0) {
                            cFloat fov = instance->getCamera()->getFov() + 1.0f;
                            instance->getCamera()->setFov(fov);
                        }
                    }
                }
            }

            void glfwWindow::windowFocusCallback(GLFWwindow* window, cInt focused)
            {

            }

        }
    }
}