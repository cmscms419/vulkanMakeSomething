#include "common.h"

namespace vkengine
{
    const std::vector<const cChar*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const cChar*> coreDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,                // 스왑체인 확장
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME         // 동적 랜더링 확장자
    };

    const std::vector<const cChar*> enabledDeviceExtensions = {};

    const std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const cString RESOURSE_PATH = "../../../../../../resource/";
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

    size_t BindingHash::operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const
    {
        size_t hash = 0;
        for (const auto& binding : bindings) {
            // Combine hash values for each field
            hash ^= std::hash<uint32_t>{}(binding.binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^=
                std::hash<uint32_t>{}(binding.descriptorType) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^=
                std::hash<uint32_t>{}(binding.descriptorCount) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<const void*>{}(binding.pImmutableSamplers) + 0x9e3779b9 + (hash << 6) +
                (hash >> 2);
        }
        return hash;
    }

    bool BindingEqual::operator()(
        const std::vector<VkDescriptorSetLayoutBinding>& lhs,
        const std::vector<VkDescriptorSetLayoutBinding>& rhs) const
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }

        for (size_t i = 0; i < lhs.size(); ++i) {
            const auto& l = lhs[i];
            const auto& r = rhs[i];

            // Note: stageFlags is not used to determine equality intentionally.

            if (l.binding != r.binding || l.descriptorType != r.descriptorType ||
                l.descriptorCount != r.descriptorCount ||
                /*  l.stageFlags != r.stageFlags || */
                l.pImmutableSamplers != r.pImmutableSamplers) {
                return false;
            }
        }
        return true;
    }

#ifdef DEBUG_
    const cBool enableValidationLayers = true;
#else
    const cBool enableValidationLayers = false;
#endif

} // namespace vkengine