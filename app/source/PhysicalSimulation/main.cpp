#include "PhysicalSimulation.h"


int main(int argc, char* argv[]) {
    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        vkengine::Log::exitToLogger("경로를 가져오는 데 실패했습니다.");
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

    config.renderGraphFilePath = "renderGraph_instance_version.json";

    std::unique_ptr<vkengine::PhysicalSimulation> app3 = std::make_unique<vkengine::PhysicalSimulation>(config, root_path);

    app3->update();
    
    return 0;
}