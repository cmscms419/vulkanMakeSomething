#include "vulkanTest/Application.h"

int main(int argc, char* argv[]) {

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "��θ� �������� �� �����߽��ϴ�." << std::endl;
    }

    std::unique_ptr<vkutil::Application> app;
    app = std::make_unique<vkutil::Application>(root_path);

    app->init();
    app->mainLoop();
    app->cleanup();

    return EXIT_SUCCESS;
}