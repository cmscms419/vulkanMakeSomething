#include "VKContext.h"

int main(int argc, char* argv[]) {

    std::vector<const char*> requiredInstanceExtensions = {};
    bool useSwapchain = false;

    vkengine::VKcontext ctx(requiredInstanceExtensions, useSwapchain);


    return EXIT_SUCCESS;
}