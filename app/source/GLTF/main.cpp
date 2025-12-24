#include "gltf_example.h"
#include "log.h"
using namespace vkengine;
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


    
    return 0;
}
