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

        GLFWwindow* window;
        bool state = true;
    };

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
}
