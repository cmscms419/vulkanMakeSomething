#include "common.h"

namespace vkengine
{
    const std::vector<const cChar*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const cChar*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,                // 스왑체인 확장
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME  // 디버깅을 위한 확장
    };

    const std::vector<const cChar*> enabledDeviceExtensions = {};

    const std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const cString RESOURSE_PATH = "../../../../../../source/";
    const cString SHADER_PATH = "../../../../../../shader/";
    const cString MODEL_PATH = "viking_room.obj";
    const cString TEXTURE_PATH = "viking_room.png";
    const cString TEST_TEXTURE_PATH = "image.png";
    const cString TEST_TEXTURE_PATH_ARRAY0 =  "Texture/512x512/Elements/Elements_15-512x512.png";
    const cString TEST_TEXTURE_PATH_ARRAY1 =  "Texture/512x512/Elements/Elements_16-512x512.png";
    const cString TEST_TEXTURE_PATH_ARRAY2 =  "Texture/512x512/Elements/Elements_17-512x512.png";
    const cString TEST_TEXTURE_PATH_ARRAY3 =  "Texture/512x512/Elements/Elements_18-512x512.png";
    const cString TEST_TEXTURE_PATH_ARRAY4 =  "Texture/512x512/Elements/Elements_19-512x512.png";
    const cString TEST_TEXTURE_PATH_ARRAY5 =  "Texture/512x512/Elements/Elements_20-512x512.png";
    const cString CUBE_TEXTURE_PATH = "Texture/skybox";

#ifdef DEBUG_
    const cBool enableValidationLayers = true;
#else
    const cBool enableValidationLayers = false;
#endif

} // namespace vkengine