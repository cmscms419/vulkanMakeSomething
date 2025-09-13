
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

    PRINT_TO_LOGGER("Hello, Vulkan!\n");
    PRINT_TO_LOGGER("Debug Mode\n");
    PRINT_TO_LOGGER("User Name: %s\n", UserName.c_str());
    PRINT_TO_LOGGER("\n");

    char path[MAX_PATH];
    std::string root_path = "";

    if (GetModuleFileNameA(NULL, path, MAX_PATH)) {
        root_path = path;
    }
    else {
        std::cerr << "��θ� �������� �� �����߽��ϴ�." << std::endl;
    }

    cString shaderComPath = root_path + "../../../../../../shader/compCSsample.spv";

    SpvReflectShaderModule module;

    // shader �б�

    std::vector<cChar> shaderCode = vkengine::helper::readSPVFile(shaderComPath);

    PRINT_TO_LOGGER("Shader Code Size: %zu bytes\n", shaderCode.size());

    // SPIR-V ��� �ʱ�ȭ
    SpvReflectResult result = spvReflectCreateShaderModule(
        shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()), &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        EXIT_TO_LOGGER("Failed to create SPIR-V module: %d\n", result);
    }


    // reflection ���� ���
    printReflectionInfo(module);

    // ��� ����
    spvReflectDestroyShaderModule(&module);

    PRINT_TO_LOGGER("\nShader reflection completed successfully!");

    return EXIT_SUCCESS;
}
