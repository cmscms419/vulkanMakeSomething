#include "moveDemo.h"
#include "log.h"

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
    ApplicationConfig config;

    // config.models.push_back(ModelConfig("DamagedHelmet.glb", "Helmet")
    //     .setTransform(
    //             glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
    //     ));

    config.models.push_back(ModelConfig("noGit/sponza/sponza.gltf", "sponza"));
    config.RanderGraphfilePath = "renderGraph.json";
        // .setTransform(
        //         glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        // ));

    std::unique_ptr<DemoApplication> app3 = std::make_unique<DemoApplication>(config, root_path);

    app3->update();
    
    return 0;
}
