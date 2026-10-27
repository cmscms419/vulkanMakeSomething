
#include "common.h"
#include "struct.h"
#include "macros.h"
#include "helper.h"
#include "log.h"

void printReflectionInfo(const SpvReflectShaderModule& reflectModule)
{
    vkengine::helper::printReflectionInfo(reflectModule);
}


int main(int argc, char* argv[]) {

    vkengine::Log::printToLogger("Hello, Vulkan!");
    vkengine::Log::printToLogger("Debug Mode");
    vkengine::Log::printToLogger("User Name: %s", UserName.c_str());
    vkengine::Log::printToLogger("");

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "경로를 가져오는 데 실패했습니다." << std::endl;
    }

    cString shaderComPath = root_path + "../../../../../../shader/compCSsample.spv";

    SpvReflectShaderModule module;

    // shader 읽기

    std::vector<cChar> shaderCode = vkengine::helper::readSPVFile(shaderComPath);

    vkengine::Log::printToLogger("Shader Code Size: %zu bytes", shaderCode.size());

    // SPIR-V 모듈 초기화
    SpvReflectResult result = spvReflectCreateShaderModule(
        shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()), &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        vkengine::Log::exitToLogger("Failed to create SPIR-V module: %d", result);
    }


    // reflection 정보 출력
    printReflectionInfo(module);

    // 모듈 정리
    spvReflectDestroyShaderModule(&module);

    vkengine::Log::printToLogger("Shader reflection completed successfully!");

    return EXIT_SUCCESS;
}
