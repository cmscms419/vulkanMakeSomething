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

    ApplicationConfig config;

    config.models.push_back(ModelConfig("3D_models/Box.obj", "Box")
        .setTransform(
                glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        ).setScale(0.2f));
    
    config.models.push_back(ModelConfig("3D_models/sphere.gltf", "Sphere")
        .setTransform(
                glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        ).setScale(0.2f));

    config.RanderGraphfilePath = "renderGraph_instance_version.json";

    std::unique_ptr<PhysicalSimulation> app3 = std::make_unique<PhysicalSimulation>(config, root_path);

    app3->update();
    
    return 0;
}