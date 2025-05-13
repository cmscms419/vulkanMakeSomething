
#include "VKengine.h"
#include "Square.h"

#define SELECTED_ENGINE 0

int main(int argc, char* argv[]) {

    _PRINT_TO_CONSOLE_("Hello, Vulkan!\n");
    _PRINT_TO_CONSOLE_("Debug Mode\n");
    _PRINT_TO_CONSOLE_("Selected Engine: %d\n", SELECTED_ENGINE);
    _PRINT_TO_CONSOLE_("User Name: %s\n", UserName.c_str());
    _PRINT_TO_CONSOLE_("\n");

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
    engine = std::make_unique<vkengine::square>(root_path);
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