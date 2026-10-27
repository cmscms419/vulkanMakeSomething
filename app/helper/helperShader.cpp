#include "helperShader.h"
#include "helperDescriptor.h"

#include "log.h"


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
                    Log::exitToLogger("Unsupported format.");
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
                    Log::exitToLogger("Failed to create SPIR-V reflection module: %d", reflectResult);
                }

                if (reflectShaderModule._internal == nullptr)
                {
                    Log::exitToLogger("SPIR-V reflection module internal structure is null.");
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
                Log::printToLogger("=== SPIR-V Shader Reflection Information ===");
                Log::printToLogger("Entry Point: ");
                Log::printToLogger(reflectModule.entry_point_name ? reflectModule.entry_point_name : "Unknown");
                Log::printToLogger("");
                Log::printToLogger("Shader Stage: %s", getShaderStageString(reflectModule.shader_stage));
                Log::printToLogger("Source Language: ");
                switch (reflectModule.source_language)
                {
                case SpvSourceLanguageGLSL:
                    Log::printToLogger("GLSL");
                    break;
                case SpvSourceLanguageHLSL:
                    Log::printToLogger("HLSL");
                    break;
                case SpvSourceLanguageOpenCL_C:
                    Log::printToLogger("OpenCL C");
                    break;
                default:
                    Log::printToLogger("Unknown (%d)", reflectModule.source_language);
                    break;
                }
                Log::printToLogger("");

                Log::printToLogger(" v%d", reflectModule.source_language_version);

                if (reflectModule.source_file)
                {
                    Log::printToLogger("Source File: %s", reflectModule.source_file);
                }

                Log::printToLogger("--- Descriptor Bindings ---");
                Log::printToLogger("Total descriptor bindings: %d", reflectModule.descriptor_binding_count);

                for (uint32_t i = 0; i < reflectModule.descriptor_binding_count; ++i)
                {
                    const SpvReflectDescriptorBinding *binding = &reflectModule.descriptor_bindings[i];
                    Log::printToLogger("  Binding %d:", i);
                    Log::printToLogger("    Name: %s", (binding->name ? binding->name : "Unknown"));
                    Log::printToLogger("    Set: %d", binding->set);
                    Log::printToLogger("    Binding: %d", binding->binding);
                    Log::printToLogger("    Type: %s", descriptor::getDescriptorTypeString(binding->descriptor_type));
                    Log::printToLogger("    Count: %d", binding->count);

                    if (binding->image.dim != SpvDimMax)
                    {
                        Log::printToLogger("    Image Dimension: ");
                        switch (binding->image.dim)
                        {
                        case SpvDim1D:
                            Log::printToLogger("1D");
                            break;
                        case SpvDim2D:
                            Log::printToLogger("2D");
                            break;
                        case SpvDim3D:
                            Log::printToLogger("3D");
                            break;
                        case SpvDimCube:
                            Log::printToLogger("Cube");
                            break;
                        case SpvDimBuffer:
                            Log::printToLogger("Buffer");
                            break;
                        default:
                            Log::printToLogger("Unknown");
                            break;
                        }
                        Log::printToLogger("    Image Format: %d", binding->image.image_format);
                    }
                }

                Log::printToLogger("--- Descriptor Sets ---");
                Log::printToLogger("Total descriptor sets: %d", reflectModule.descriptor_set_count);
                for (uint32_t i = 0; i < reflectModule.descriptor_set_count; ++i)
                {
                    const SpvReflectDescriptorSet *set = &reflectModule.descriptor_sets[i];
                    Log::printToLogger("  Set %d: %d bindings", set->set, set->binding_count);
                }

                Log::printToLogger("--- Input Variables ---");
                Log::printToLogger("Total input variables: %d", reflectModule.input_variable_count);
                for (uint32_t i = 0; i < reflectModule.input_variable_count; ++i)
                {
                    const SpvReflectInterfaceVariable *var = reflectModule.input_variables[i];
                    Log::printToLogger("  Input %d: %s", i, (var->name ? var->name : "Unknown"));
                    Log::printToLogger("    Location: %d", var->location);
                }

                Log::printToLogger("--- Output Variables ---");
                Log::printToLogger("Total output variables: %d", reflectModule.output_variable_count);
                for (uint32_t i = 0; i < reflectModule.output_variable_count; ++i)
                {
                    const SpvReflectInterfaceVariable *var = reflectModule.output_variables[i];
                    Log::printToLogger("  Output %d: %s", i, (var->name ? var->name : "Unknown"));
                    Log::printToLogger("    Location: %d", var->location);
                }

                Log::printToLogger("--- Push Constants ---");
                Log::printToLogger("Total push constant blocks: %d", reflectModule.push_constant_block_count);
                for (uint32_t i = 0; i < reflectModule.push_constant_block_count; ++i)
                {
                    const SpvReflectBlockVariable *block = &reflectModule.push_constant_blocks[i];
                    Log::printToLogger("  Push constant block %d:", i);
                    Log::printToLogger("    Name: %s", (block->name ? block->name : "Unknown"));
                    Log::printToLogger("    Size: %d bytes", block->size);
                    Log::printToLogger("    Offset: %d", block->offset);
                }

                // For compute shaders, show workgroup size
                if (reflectModule.shader_stage & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT)
                {
                    Log::printToLogger("--- Compute Shader Info ---");
                    Log::printToLogger("Local workgroup size: (%d, %d, %d)",
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
                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
                    return VK_FORMAT_R32G32B32A32_SINT;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                default:
                    Log::exitToLogger("Unsupported SPIR-V format");
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

            cString convertShaderStageFlagsToString(VkShaderStageFlags flags)
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