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

    std::unique_ptr<vkengine::VulkanEngine> engine;
    
    switch (0)
    {
    case 0:
        // �⺻��
        engine = std::make_unique<vkengine::VulkanEngine>(root_path);
        break;
    case 1:
        // �ﰢ��

        break;
    default:
        break;
    }

    engine->init();

    engine->mainLoop();

    return EXIT_SUCCESS;
}