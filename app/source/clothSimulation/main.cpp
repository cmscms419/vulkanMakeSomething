#include "Cloth.h"
#include "log.h"


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

    // 바닥. 천은 origin (0, 2, 0) 에 매달려 y = 0.4 까지 내려온다.
    // 바닥을 y = 0 에 두면 천이 바닥에 닿기 전에 멈춘다.
    config.models.push_back(ModelConfig("3D_models/box.obj", "Floor")
        .setTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, -5.0f, 0.0f))
        ).setScale(1.0f));

    config.renderGraphFilePath = "renderGraph_cloth.json";

    std::unique_ptr<vkengine::Cloth> app = std::make_unique<vkengine::Cloth>(config, root_path);

    app->update();

    return 0;
}
