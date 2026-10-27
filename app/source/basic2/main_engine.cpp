#include "Context.h"

int main(int argc, char* argv[]) {

    std::vector<const char*> requiredInstanceExtensions = {};
    bool useSwapchain = false;

    vkengine::Context ctx(requiredInstanceExtensions, useSwapchain);


    return EXIT_SUCCESS;
}