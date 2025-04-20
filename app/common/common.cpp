#include "common.h"

namespace vkengine
{
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME
    };

    const std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const std::string RESOURSE_PATH = "../../../../../../source/";

    const std::string MODEL_PATH = "viking_room.obj";
    const std::string TEXTURE_PATH = "viking_room.png";
    const std::string TEST_TEXTURE_PATH = "image.png";

    const std::string TEST_TEXTURE_PATH_ARRAY0 =  "Texture/512x512/Elements/Elements_15-512x512.png";
    const std::string TEST_TEXTURE_PATH_ARRAY1 =  "Texture/512x512/Elements/Elements_16-512x512.png";
    const std::string TEST_TEXTURE_PATH_ARRAY2 =  "Texture/512x512/Elements/Elements_17-512x512.png";
    const std::string TEST_TEXTURE_PATH_ARRAY3 =  "Texture/512x512/Elements/Elements_18-512x512.png";
    const std::string TEST_TEXTURE_PATH_ARRAY4 =  "Texture/512x512/Elements/Elements_19-512x512.png";
    const std::string TEST_TEXTURE_PATH_ARRAY5 =  "Texture/512x512/Elements/Elements_20-512x512.png";

    const std::string CUBE_TEXTURE_PATH = "Texture/skybox";
    


#ifdef DEBUG_
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

} // namespace vkengine