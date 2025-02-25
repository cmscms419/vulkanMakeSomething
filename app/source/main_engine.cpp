#include "engine/VKengine.h"
#include "../../app/cpp/triangle.h"

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
    
    switch (1)
    {
    case 0:
        // 기본형
        engine = std::make_unique<vkengine::VulkanEngine>(root_path);
        break;
    case 1:
        // 삼각형
        engine = std::make_unique<vkengine::triangle>(root_path);
        break;
    default:
        break;
    }

    engine->init();

    engine->prepare();

    engine->mainLoop();

    engine->cleanup();

    return EXIT_SUCCESS;
}