#ifndef  INCLUDE_VK_WINDOWS_H_
#define  INCLUDE_VK_WINDOWS_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include <GLFW/glfw3.h>

namespace vkengine {
    namespace platform {

        // Window �̺�Ʈ �ݹ� Ÿ�Ե�
        using WindowResizeCallback = std::function<void(cInt width, cInt height)>;
        using WindowCloseCallback = std::function<cBool()>; // true�� â �ݱ� ���
        using KeyCallback = std::function<void(cInt key, cInt scancode, cInt action, cInt mods)>;
        using MouseButtonCallback = std::function<void(cInt button, cInt action, cInt mods)>;
        using MouseMoveCallback = std::function<void(cDouble xpos, cDouble ypos)>;
        using ScrollCallback = std::function<void(cDouble xoffset, cDouble yoffset)>;
        using WindowFocusCallback = std::function<void(cBool focused)>;

        // Window ���� �ɼ�
        struct WindowCreateInfo {
            cInt width = WIDTH;
            cInt height = HEIGHT;
            cString title = "Vulkan Game Window";
            cBool resizable = true;
            cBool visible = true;
            cBool decorated = true;
            cBool focused = true;
            cBool maximized = false;
            cBool vsync = true;
        };

        class VirtualWindows {
        public:
            virtual ~VirtualWindows() = default;
            
            // ������ ����
            virtual cBool createWindow(const WindowCreateInfo& info) = 0;
            
            // ������ �̺�Ʈ ó��
            virtual void pollEvents() = 0;
            virtual void waitEvents() = 0;
            
            // ������ ���� Ȯ��
            virtual cBool shouldClose() = 0;
            virtual void setShouldClose(cBool value) = 0;
            
            // ������ ũ�� ��������
            virtual void getFramebufferSize(int* width, int* height) = 0;
            
            // �ݹ� ����
            virtual void setWindowResizeCallback(WindowResizeCallback callback) = 0;
            virtual void setWindowCloseCallback(WindowCloseCallback callback) = 0;
            virtual void setKeyCallback(KeyCallback callback) = 0;
            virtual void setMouseButtonCallback(MouseButtonCallback callback) = 0;
            virtual void setMouseMoveCallback(MouseMoveCallback callback) = 0;
            virtual void setScrollCallback(ScrollCallback callback) = 0;
            virtual void setWindowFocusCallback(WindowFocusCallback callback) = 0;
            virtual VkSurfaceKHR createVulkanSurface(VkInstance instance) = 0;
        };

        class BaseWindow : public VirtualWindows {
        public:
            BaseWindow() = default;
            virtual ~BaseWindow() = default;

            void setWindowResizeCallback(WindowResizeCallback callback) override;
            void setWindowCloseCallback(WindowCloseCallback callback) override;
            void setKeyCallback(KeyCallback callback) override;
            void setMouseButtonCallback(MouseButtonCallback callback) override;
            void setMouseMoveCallback(MouseMoveCallback callback) override;
            void setScrollCallback(ScrollCallback callback) override;
            void setWindowFocusCallback(WindowFocusCallback callback) override;

            virtual VkSurfaceKHR createVulkanSurface(VkInstance instance) = 0;

            cBool isCreated() const { return m_isCreated; }
            const WindowCreateInfo& getCreateInfo() const { return m_createInfo; }

        protected:
            WindowCreateInfo m_createInfo;
            cBool m_isCreated = false;

            // �ݹ� �Լ���
            WindowResizeCallback m_resizeCallback;
            WindowCloseCallback m_closeCallback;
            KeyCallback m_keyCallback;
            MouseButtonCallback m_mouseButtonCallback;
            MouseMoveCallback m_mouseMoveCallback;
            ScrollCallback m_scrollCallback;
            WindowFocusCallback m_focusCallback;

        };

        // Window Factory
        class WindowFactory {
        public:
            enum class Backend {
                GLFW,
                // ���� SDL, Win32 �� �߰� ����
            };

            static std::unique_ptr<VirtualWindows> create(Backend backend = Backend::GLFW);
        };

        // Window Manager
        class WindowManager {
        private:
            static WindowManager* s_instance;
            std::vector<std::weak_ptr<VirtualWindows>> m_windows;

        public:
            static WindowManager& getInstance();
            void registerWindow(std::shared_ptr<VirtualWindows> window);
            void pollAllEvents();
            void cleanup();
        };

        namespace glfw {
            
            class glfwWindow : public BaseWindow {
            public:
                glfwWindow();
                ~glfwWindow() override;

                // glfw ������ ����
                bool createWindow(const WindowCreateInfo& createInfo) override;

                // glfw ������ ���� Ȯ��
                void pollEvents() override;
                void waitEvents() override;

                // ������ ���� Ȯ�� �޼ҵ�� ����
                cBool shouldClose() override;
                void setShouldClose(cBool value) override;

                // ������ ũ�� ��������
                void getFramebufferSize(int* width, int* height) override;

                // VKsurfaceKHR ����
                VkSurfaceKHR createVulkanSurface(VkInstance instance) override;


            private:
                GLFWwindow* m_window = nullptr;

                // ���� �ݹ� �Լ��� (GLFW C API��)
                static void framebufferSizeCallback(GLFWwindow* window, cInt width, cInt height);
                static void windowCloseCallback(GLFWwindow* window);
                static void keyCallback(GLFWwindow* window, cInt key, cInt scancode, cInt action, cInt mods);
                static void mouseButtonCallback(GLFWwindow* window, cInt button, cInt action, cInt mods);
                static void cursorPosCallback(GLFWwindow* window, cDouble xpos, cDouble ypos);
                static void scrollCallback(GLFWwindow* window, cDouble xoffset, cDouble yoffset);
                static void windowFocusCallback(GLFWwindow* window, cInt focused);
            };

        }
    }
}

#endif // ! INCLUDE_VK_WINDOWS_H_
