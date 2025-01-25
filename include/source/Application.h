#pragma once
#define VKUTIL_APPLICATION_H

#include <cms491_common.h>

constexpr int MAX_FRAMES = 4;

#define shaderRootPath "shader/"

namespace vkutil {

    class Application {
    public:

        virtual void init();
        virtual bool run();
        virtual void update();
        virtual void setup();
        virtual void cleanup();


    private:
        virtual void initWindow();
        virtual void initVulkan();
        virtual void mainLoop();
        void createInstance();

        GLFWwindow* window = nullptr; // GLFW 윈도우
        VkInstance VKinstance = {}; // Vulkan 인스턴스


        bool state = true; // 프로그램 상태
    };

}
