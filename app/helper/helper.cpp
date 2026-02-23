#include "helper.h"

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {

        cString shaderStageFlagsToString(VkShaderStageFlags flags)
        {
            std::string result;
            if (flags & VK_SHADER_STAGE_VERTEX_BIT)
                result += "VERTEX|";
            if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
                result += "TESSELLATION_CONTROL|";
            if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                result += "TESSELLATION_EVALUATION|";
            if (flags & VK_SHADER_STAGE_GEOMETRY_BIT)
                result += "GEOMETRY|";
            if (flags & VK_SHADER_STAGE_FRAGMENT_BIT)
                result += "FRAGMENT|";
            if (flags & VK_SHADER_STAGE_COMPUTE_BIT)
                result += "COMPUTE|";
            if (flags & VK_SHADER_STAGE_RAYGEN_BIT_KHR)
                result += "RAYGEN|";
            if (flags & VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
                result += "ANY_HIT|";
            if (flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
                result += "CLOSEST_HIT|";
            if (flags & VK_SHADER_STAGE_MISS_BIT_KHR)
                result += "MISS|";
            if (flags & VK_SHADER_STAGE_INTERSECTION_BIT_KHR)
                result += "INTERSECTION|";
            if (flags & VK_SHADER_STAGE_CALLABLE_BIT_KHR)
                result += "CALLABLE|";
            if (flags & VK_SHADER_STAGE_TASK_BIT_EXT)
                result += "TASK|";
            if (flags & VK_SHADER_STAGE_MESH_BIT_EXT)
                result += "MESH|";
            if (!result.empty())
                result.pop_back(); // Remove trailing '|'
            return result.empty() ? "NONE" : result;
        }

        cUint32_t getFormatSize(VkFormat format)
        {
            switch (format)
            {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R8_SINT:
                return 1;
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R8G8_SINT:
                return 2;
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_R8G8B8_SINT:
                return 3;
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_R32_SFLOAT:
                return 4;
            case VK_FORMAT_R32G32_SFLOAT:
                return 8;
            case VK_FORMAT_R32G32B32_SFLOAT:
                return 12;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 16;
            case VK_FORMAT_R32_SINT:
                return 4;
            case VK_FORMAT_R32G32_SINT:
                return 8;
            case VK_FORMAT_R32G32B32_SINT:
                return 12;
            case VK_FORMAT_R32G32B32A32_SINT:
                return 16;
            case VK_FORMAT_R32_UINT:
                return 4;
            case VK_FORMAT_R32G32_UINT:
                return 8;
            case VK_FORMAT_R32G32B32_UINT:
                return 12;
            case VK_FORMAT_R32G32B32A32_UINT:
                return 16;
            default:
                EXIT_TO_LOGGER("Unsupported format.");
                return 0; // Unknown/unsupported format
            }
        }

        VkFormat getVkFormatFromSpvReflectFormat(SpvReflectFormat format)
        {
            switch (format)
            {
            case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
                return VK_FORMAT_R32G32_SFLOAT;
            case SPV_REFLECT_FORMAT_R32_SFLOAT:
                return VK_FORMAT_R32_SFLOAT;
            default:
                EXIT_TO_LOGGER("Unsupported SPIR-V format");
                return VK_FORMAT_UNDEFINED; // Unsupported or unknown format
            }
        }

        void printReflectionInfo(const SpvReflectShaderModule &reflectModule)
        {
            PRINT_TO_LOGGER("=== SPIR-V Shader Reflection Information ===\n");
            PRINT_TO_LOGGER("Entry Point: ");
            PRINT_TO_LOGGER(reflectModule.entry_point_name ? reflectModule.entry_point_name : "Unknown\n");
            PRINT_TO_LOGGER("\n");
            PRINT_TO_LOGGER("Shader Stage: %s\n", vkengine::helper::getShaderStageString(reflectModule.shader_stage));
            PRINT_TO_LOGGER("Source Language: ");
            switch (reflectModule.source_language)
            {
            case SpvSourceLanguageGLSL:
                PRINT_TO_LOGGER("GLSL");
                break;
            case SpvSourceLanguageHLSL:
                PRINT_TO_LOGGER("HLSL");
                break;
            case SpvSourceLanguageOpenCL_C:
                PRINT_TO_LOGGER("OpenCL C");
                break;
            default:
                PRINT_TO_LOGGER("Unknown (%d)", reflectModule.source_language);
                break;
            }
            PRINT_TO_LOGGER("\n");

            PRINT_TO_LOGGER(" v%d\n", reflectModule.source_language_version);

            if (reflectModule.source_file)
            {
                PRINT_TO_LOGGER("Source File: %s\n", reflectModule.source_file);
            }

            PRINT_TO_LOGGER("\n--- Descriptor Bindings ---\n");
            PRINT_TO_LOGGER("Total descriptor bindings: %d\n", reflectModule.descriptor_binding_count);

            for (uint32_t i = 0; i < reflectModule.descriptor_binding_count; ++i)
            {
                const SpvReflectDescriptorBinding *binding = &reflectModule.descriptor_bindings[i];
                PRINT_TO_LOGGER("  Binding %d:\n", i);
                PRINT_TO_LOGGER("    Name: %s\n", (binding->name ? binding->name : "Unknown"));
                PRINT_TO_LOGGER("    Set: %d\n", binding->set);
                PRINT_TO_LOGGER("    Binding: %d\n", binding->binding);
                PRINT_TO_LOGGER("    Type: %s\n", vkengine::helper::getDescriptorTypeString(binding->descriptor_type));
                PRINT_TO_LOGGER("    Count: %d\n", binding->count);

                if (binding->image.dim != SpvDimMax)
                {
                    PRINT_TO_LOGGER("    Image Dimension: \n");
                    switch (binding->image.dim)
                    {
                    case SpvDim1D:
                        PRINT_TO_LOGGER("1D");
                        break;
                    case SpvDim2D:
                        PRINT_TO_LOGGER("2D");
                        break;
                    case SpvDim3D:
                        PRINT_TO_LOGGER("3D");
                        break;
                    case SpvDimCube:
                        PRINT_TO_LOGGER("Cube");
                        break;
                    case SpvDimBuffer:
                        PRINT_TO_LOGGER("Buffer");
                        break;
                    default:
                        PRINT_TO_LOGGER("Unknown");
                        break;
                    }
                    PRINT_TO_LOGGER("    Image Format: %d", binding->image.image_format);
                }
            }

            PRINT_TO_LOGGER("\n--- Descriptor Sets ---\n");
            PRINT_TO_LOGGER("Total descriptor sets: %d\n", reflectModule.descriptor_set_count);
            for (uint32_t i = 0; i < reflectModule.descriptor_set_count; ++i)
            {
                const SpvReflectDescriptorSet *set = &reflectModule.descriptor_sets[i];
                PRINT_TO_LOGGER("  Set %d: %d bindings\n", set->set, set->binding_count);
            }

            PRINT_TO_LOGGER("\n--- Input Variables ---\n");
            PRINT_TO_LOGGER("Total input variables: %d\n", reflectModule.input_variable_count);
            for (uint32_t i = 0; i < reflectModule.input_variable_count; ++i)
            {
                const SpvReflectInterfaceVariable *var = reflectModule.input_variables[i];
                PRINT_TO_LOGGER("  Input %d: %s\n", i, (var->name ? var->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Location: %d\n", var->location);
            }

            PRINT_TO_LOGGER("\n--- Output Variables ---\n");
            PRINT_TO_LOGGER("Total output variables: %d\n", reflectModule.output_variable_count);
            for (uint32_t i = 0; i < reflectModule.output_variable_count; ++i)
            {
                const SpvReflectInterfaceVariable *var = reflectModule.output_variables[i];
                PRINT_TO_LOGGER("  Output %d: %s\n", i, (var->name ? var->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Location: %d\n", var->location);
            }

            PRINT_TO_LOGGER("\n--- Push Constants ---\n");
            PRINT_TO_LOGGER("Total push constant blocks: %d\n", reflectModule.push_constant_block_count);
            for (uint32_t i = 0; i < reflectModule.push_constant_block_count; ++i)
            {
                const SpvReflectBlockVariable *block = &reflectModule.push_constant_blocks[i];
                PRINT_TO_LOGGER("  Push constant block %d:\n", i);
                PRINT_TO_LOGGER("    Name: %s\n", (block->name ? block->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Size: %d bytes\n", block->size);
                PRINT_TO_LOGGER("    Offset: %d\n", block->offset);
            }

            // For compute shaders, show workgroup size
            if (reflectModule.shader_stage & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT)
            {
                PRINT_TO_LOGGER("\n--- Compute Shader Info ---\n");
                PRINT_TO_LOGGER("Local workgroup size: (%d, %d, %d)\n",
                                reflectModule.entry_points[0].local_size.x,
                                reflectModule.entry_points[0].local_size.y,
                                reflectModule.entry_points[0].local_size.z);
            }
        }

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

        const cChar *getShaderStageString(const SpvReflectShaderStageFlagBits &stage)
        {
            switch (stage)
            {
            case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                return "Vertex";
            case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                return "Tessellation Control";
            case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                return "Tessellation Evaluation";
            case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                return "Geometry";
            case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                return "Fragment";
            case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
                return "Compute";
            default:
                return "Unknown";
            }
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

        cBool hasStencilComponent(VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

    }
}