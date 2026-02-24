#include "helperShader.h"
#include "helperDescriptor.h"

#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {
        namespace shader
        {

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

            SpvReflectShaderModule createSpvReflectModule(const std::vector<cChar> &code)
            {
                SpvReflectShaderModule reflectShaderModule;
                SpvReflectResult reflectResult = spvReflectCreateShaderModule(
                    code.size(),
                    reinterpret_cast<const cUint32_t *>(code.data()),
                    &reflectShaderModule);
                if (reflectResult != SPV_REFLECT_RESULT_SUCCESS)
                {
                    EXIT_TO_LOGGER("Failed to create SPIR-V reflection module: %d", reflectResult);
                }

                if (reflectShaderModule._internal == nullptr)
                {
                    EXIT_TO_LOGGER("SPIR-V reflection module internal structure is null.");
                }

                return reflectShaderModule;
            }

            VkShaderModule createShaderModule(VkDevice device, const std::vector<cChar> &code)
            {
                VkShaderModuleCreateInfo createInfo{};

                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = code.size();
                createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

                VkShaderModule shaderModule{VK_NULL_HANDLE};

                _VK_CHECK_RESULT_(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

                return shaderModule;
            }

            void printReflectionInfo(const SpvReflectShaderModule &reflectModule)
            {
                PRINT_TO_LOGGER("=== SPIR-V Shader Reflection Information ===\n");
                PRINT_TO_LOGGER("Entry Point: ");
                PRINT_TO_LOGGER(reflectModule.entry_point_name ? reflectModule.entry_point_name : "Unknown\n");
                PRINT_TO_LOGGER("\n");
                PRINT_TO_LOGGER("Shader Stage: %s\n", getShaderStageString(reflectModule.shader_stage));
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
                    PRINT_TO_LOGGER("    Type: %s\n", descriptor::getDescriptorTypeString(binding->descriptor_type));
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

        }
    }
}