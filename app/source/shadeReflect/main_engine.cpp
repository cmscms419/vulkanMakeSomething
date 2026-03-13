
#include "common.h"
#include "struct.h"
#include "macros.h"
#include "helper.h"
#include "log.h"

using namespace vkengine::Log;
using namespace std;
void printReflectionInfo(const SpvReflectShaderModule& reflectModule)
{
    vkengine::helper::printReflectionInfo(reflectModule);
}


int main(int argc, char* argv[]) {

    PRINT_TO_LOGGER("Hello, Vulkan!");
    PRINT_TO_LOGGER("Debug Mode");
    PRINT_TO_LOGGER("User Name: %s", UserName.c_str());
    PRINT_TO_LOGGER("");

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

    PRINT_TO_LOGGER("Shader Code Size: %zu bytes", shaderCode.size());

    // SPIR-V 모듈 초기화
    SpvReflectResult result = spvReflectCreateShaderModule(
        shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()), &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        EXIT_TO_LOGGER("Failed to create SPIR-V module: %d", result);
    }


    // reflection 정보 출력
    printReflectionInfo(module);

    // 모듈 정리
    spvReflectDestroyShaderModule(&module);

    PRINT_TO_LOGGER("Shader reflection completed successfully!");

    return EXIT_SUCCESS;
}
