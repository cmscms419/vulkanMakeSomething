#include "engine/VKengine.h"

int main(int argc, char* argv[]) {

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "경로를 가져오는 데 실패했습니다." << std::endl;
    }

    std::unique_ptr<vkengine::VulkanEngine> engine;
    
    switch (0)
    {
    case 0:
        // 기본형
        engine = std::make_unique<vkengine::VulkanEngine>(root_path);
        break;
    case 1:
        // 삼각형

        break;
    default:
        break;
    }

    engine->init();

    engine->mainLoop();

    return EXIT_SUCCESS;
}