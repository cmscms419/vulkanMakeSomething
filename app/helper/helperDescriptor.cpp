#include "helperDescriptor.h"

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {
        namespace descriptor
        {

            cString descriptorTypeToString(VkDescriptorType type)
            {
                cString str;

                switch (type)
                {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    str = "SAMPLER";
                    break;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    str = "COMBINED_IMAGE_SAMPLER";
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    str = "SAMPLED_IMAGE";
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    str = "STORAGE_IMAGE";
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    str = "UNIFORM_TEXEL_BUFFER";
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    str = "STORAGE_TEXEL_BUFFER";
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    str = "UNIFORM_BUFFER";
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    str = "STORAGE_BUFFER";
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    str = "UNIFORM_BUFFER_DYNAMIC";
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    str = "STORAGE_BUFFER_DYNAMIC";
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    str = "INPUT_ATTACHMENT";
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                    str = "INLINE_UNIFORM_BLOCK";
                    break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                    str = "ACCELERATION_STRUCTURE_KHR";
                    break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                    str = "ACCELERATION_STRUCTURE_NV";
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                    str = "SAMPLE_WEIGHT_IMAGE_QCOM";
                    break;
                case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                    str = "BLOCK_MATCH_IMAGE_QCOM";
                    break;
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    str = "MUTABLE_EXT";
                    break;
                default:
                    str = "UNKNOWN_DESCRIPTOR_TYPE";
                    break;
                }

                return str;
            }

            VkDescriptorType stringToDescriptorType(const cString &typeStr)
            {
                static const std::unordered_map<cString, VkDescriptorType> stringToTypeMap = {
                    {"SAMPLER", VK_DESCRIPTOR_TYPE_SAMPLER},
                    {"COMBINED_IMAGE_SAMPLER", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
                    {"SAMPLED_IMAGE", VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
                    {"STORAGE_IMAGE", VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
                    {"UNIFORM_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
                    {"STORAGE_TEXEL_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
                    {"UNIFORM_BUFFER", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
                    {"STORAGE_BUFFER", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
                    {"UNIFORM_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
                    {"STORAGE_BUFFER_DYNAMIC", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
                    {"INPUT_ATTACHMENT", VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},
                    {"INLINE_UNIFORM_BLOCK", VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK},
                    {"ACCELERATION_STRUCTURE_KHR", VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR},
                    {"ACCELERATION_STRUCTURE_NV", VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV},
                    {"SAMPLE_WEIGHT_IMAGE_QCOM", VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM},
                    {"BLOCK_MATCH_IMAGE_QCOM", VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM},
                    {"MUTABLE_EXT", VK_DESCRIPTOR_TYPE_MUTABLE_EXT}};

                auto it = stringToTypeMap.find(typeStr);
                if (it != stringToTypeMap.end())
                {
                    return it->second;
                }

                EXIT_TO_LOGGER("Error: Unknown descriptor type string: %s\n", typeStr.c_str());
                return VK_DESCRIPTOR_TYPE_MAX_ENUM; // Return a default value in case of error
            }

            const cChar *getDescriptorTypeString(SpvReflectDescriptorType type)
            {
                switch (type)
                {
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    return "Sampler";
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    return "Combined Image Sampler";
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    return "Sampled Image";
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    return "Storage Image";
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    return "Uniform Texel Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    return "Storage Texel Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    return "Uniform Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    return "Storage Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    return "Dynamic Uniform Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    return "Dynamic Storage Buffer";
                case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    return "Input Attachment";
                default:
                    return "Unknown";
                }
            }

        }
    }
}