#include "engine/VKengine.h"
#include "../../app/cpp/triangle.h"

int main(int argc, char* argv[]) {

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "��θ� �������� �� �����߽��ϴ�." << std::endl;
    }

    std::unique_ptr<vkengine::VulkanEngine> engine;
    
    switch (1)
    {
    case 0:
        // �⺻��
        engine = std::make_unique<vkengine::VulkanEngine>(root_path);
        break;
    case 1:
        // �ﰢ��
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