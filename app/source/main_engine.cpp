#include "engine/VKengine.h"

int main(int argc, char* argv[]) {

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "��θ� �������� �� �����߽��ϴ�." << std::endl;
    }

    vkengine::VulkanEngine engine;

    engine.init();

    engine.mainLoop();

    engine.cleanup();

    return EXIT_SUCCESS;
}