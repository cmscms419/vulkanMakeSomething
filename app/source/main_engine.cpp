#include "engine/VKengine.h"
#include "../../app/cpp/triangle.h"
#include "../../app/cpp/cameraEngine.h"

#define SELECTED_ENGINE 2

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


#if SELECTED_ENGINE == 0
        engine = std::make_unique<vkengine::VulkanEngine>(root_path);

#elif SELECTED_ENGINE == 1
        engine = std::make_unique<vkengine::triangle>(root_path);
#elif SELECTED_ENGINE == 2
        engine = std::make_unique<vkengine::cameraEngine>(root_path);
#else
    return EXIT_FAILURE;
#endif

#if SELECTED_ENGINE  < 0

#else
    engine->init();

    engine->prepare();

    engine->mainLoop();

    engine->cleanup();
#endif

    return EXIT_SUCCESS;
}