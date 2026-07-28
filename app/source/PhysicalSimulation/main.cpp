#include "PhysicalSimulation.h"

using namespace vkengine::Log;
using namespace vkengine;

int main(int argc, char* argv[]) {
    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        EXIT_TO_LOGGER("경로를 가져오는 데 실패했습니다.");
    }

    std::unique_ptr<PhysicalSimulation> app3 = std::make_unique<PhysicalSimulation>(root_path);

    app3->update();
    
    return 0;
}