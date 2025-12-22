#include "helper.h"
#include "helperCreate.h"

using namespace vkengine::Log;

namespace vkengine {
    namespace helper {

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
            switch (format) {
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
            switch (format) {
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

        void printReflectionInfo(const SpvReflectShaderModule& reflectModule)
        {
            PRINT_TO_LOGGER("=== SPIR-V Shader Reflection Information ===\n");
            PRINT_TO_LOGGER("Entry Point: ");
            PRINT_TO_LOGGER(reflectModule.entry_point_name ? reflectModule.entry_point_name : "Unknown\n");
            PRINT_TO_LOGGER("\n");
            PRINT_TO_LOGGER("Shader Stage: %s\n", vkengine::helper::getShaderStageString(reflectModule.shader_stage));
            PRINT_TO_LOGGER("Source Language: ");
            switch (reflectModule.source_language) {
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

            if (reflectModule.source_file) {
                PRINT_TO_LOGGER("Source File: %s\n", reflectModule.source_file);
            }

            PRINT_TO_LOGGER("\n--- Descriptor Bindings ---\n");
            PRINT_TO_LOGGER("Total descriptor bindings: %d\n", reflectModule.descriptor_binding_count);

            for (uint32_t i = 0; i < reflectModule.descriptor_binding_count; ++i) {
                const SpvReflectDescriptorBinding* binding = &reflectModule.descriptor_bindings[i];
                PRINT_TO_LOGGER("  Binding %d:\n", i);
                PRINT_TO_LOGGER("    Name: %s\n", (binding->name ? binding->name : "Unknown"));
                PRINT_TO_LOGGER("    Set: %d\n", binding->set);
                PRINT_TO_LOGGER("    Binding: %d\n", binding->binding);
                PRINT_TO_LOGGER("    Type: %s\n", vkengine::helper::getDescriptorTypeString(binding->descriptor_type));
                PRINT_TO_LOGGER("    Count: %d\n", binding->count);

                if (binding->image.dim != SpvDimMax) {
                    PRINT_TO_LOGGER("    Image Dimension: \n");
                    switch (binding->image.dim) {
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
            for (uint32_t i = 0; i < reflectModule.descriptor_set_count; ++i) {
                const SpvReflectDescriptorSet* set = &reflectModule.descriptor_sets[i];
                PRINT_TO_LOGGER("  Set %d: %d bindings\n", set->set, set->binding_count);
            }

            PRINT_TO_LOGGER("\n--- Input Variables ---\n");
            PRINT_TO_LOGGER("Total input variables: %d\n", reflectModule.input_variable_count);
            for (uint32_t i = 0; i < reflectModule.input_variable_count; ++i) {
                const SpvReflectInterfaceVariable* var = reflectModule.input_variables[i];
                PRINT_TO_LOGGER("  Input %d: %s\n", i, (var->name ? var->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Location: %d\n", var->location);
            }

            PRINT_TO_LOGGER("\n--- Output Variables ---\n");
            PRINT_TO_LOGGER("Total output variables: %d\n", reflectModule.output_variable_count);
            for (uint32_t i = 0; i < reflectModule.output_variable_count; ++i) {
                const SpvReflectInterfaceVariable* var = reflectModule.output_variables[i];
                PRINT_TO_LOGGER("  Output %d: %s\n", i, (var->name ? var->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Location: %d\n", var->location);
            }

            PRINT_TO_LOGGER("\n--- Push Constants ---\n");
            PRINT_TO_LOGGER("Total push constant blocks: %d\n", reflectModule.push_constant_block_count);
            for (uint32_t i = 0; i < reflectModule.push_constant_block_count; ++i) {
                const SpvReflectBlockVariable* block = &reflectModule.push_constant_blocks[i];
                PRINT_TO_LOGGER("  Push constant block %d:\n", i);
                PRINT_TO_LOGGER("    Name: %s\n", (block->name ? block->name : "Unknown\n"));
                PRINT_TO_LOGGER("    Size: %d bytes\n", block->size);
                PRINT_TO_LOGGER("    Offset: %d\n", block->offset);
            }

            // For compute shaders, show workgroup size
            if (reflectModule.shader_stage & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT) {
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

            switch (type) {
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

        VkDescriptorType stringToDescriptorType(const cString& typeStr)
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
         {"MUTABLE_EXT", VK_DESCRIPTOR_TYPE_MUTABLE_EXT} };

            auto it = stringToTypeMap.find(typeStr);
            if (it != stringToTypeMap.end()) {
                return it->second;
            }

            EXIT_TO_LOGGER("Error: Unknown descriptor type string: %s\n", typeStr.c_str());
            return VK_DESCRIPTOR_TYPE_MAX_ENUM; // Return a default value in case of error

        }

        const cChar* getShaderStageString(const SpvReflectShaderStageFlagBits& stage)
        {
            switch (stage) {
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

        const cChar* getDescriptorTypeString(SpvReflectDescriptorType type)
        {
            switch (type) {
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

        cString getPhysicalDeviceTypeString(VkPhysicalDeviceType type)
        {
            switch (type) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                return "Other";
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "Integrated GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "Discrete GPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return "Virtual GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "CPU";
            default:
                return "Unknown";
            }
        }

        VkAccessFlags getFromOldLayoutToVkAccessFlags(VkImageLayout format)
        {

            VkAccessFlags accessFlags = 0;

            switch (format)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                // or 깊이/스텐실 이미지 초기화 경우로, 이전 레이아웃이 사용 불가능한 상태에서 시작함
                accessFlags = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

            return accessFlags;
        }

        VkAccessFlags getFromNewLayoutToVkAccessFlags(VkImageLayout format)
        {

            VkAccessFlags accessFlags = 0;

            switch (format)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                accessFlags = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // 전송 완료 후 색상 첨부 최적화 레이아웃으로 전환
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 쓰기 작업을 위한 쓰기 액세스 허용 설정
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }
            return accessFlags;
        }

        uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        size_t alignedSize(size_t value, size_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        VkDeviceSize alignedVkSize(VkDeviceSize value, VkDeviceSize alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        void initializeSynchronization(VkDevice device, cUint32_t maxFramesInFlight, cUint32_t imageCount, std::vector<VkSemaphore>& presentSemaphores, std::vector<VkSemaphore>& renderSemaphores, std::vector<VkFence>& inFlightFences)
        {
            presentSemaphores.resize(imageCount);
            renderSemaphores.resize(imageCount);
            inFlightFences.resize(maxFramesInFlight);

            for (cUint32_t i = 0; i < imageCount; i++) {
                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                _VK_CHECK_RESULT_(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &presentSemaphores[i]));
                _VK_CHECK_RESULT_(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderSemaphores[i]));
            }

            for (cUint32_t i = 0; i < maxFramesInFlight; i++) {
                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                _VK_CHECK_RESULT_(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
            }
        }

        void cleanupSynchronization(VkDevice device, std::vector<VkSemaphore>& presentSemaphores, std::vector<VkSemaphore>& renderSemaphores, std::vector<VkFence>& inFlightFences)
        {
            for (auto& semaphore : presentSemaphores) {
                vkDestroySemaphore(device, semaphore, nullptr);
            }
            for (auto& semaphore : renderSemaphores) {
                vkDestroySemaphore(device, semaphore, nullptr);
            }
            for (auto& fence : inFlightFences) {
                vkDestroyFence(device, fence, nullptr);
            }
        }

        cBool isDeviceSuitableWithSurface(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices& indices)
        {
            QueueFamilyIndices indices_ = findQueueFamiliesWitchSurface(device, VKsurface);
            indices = indices_;

            cBool extensionsSupported = checkDeviceExtensionSupport(device);
            cBool swapChainAdequate = false;

            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, VKsurface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }

        cBool isDeviceSuitableWithoutSurface(VkPhysicalDevice device, QueueFamilyIndices2& indices)
        {
            // 1. 큐 패밀리 검사 (Surface 없이 그래픽스 큐만 확인)
            QueueFamilyIndices2 indices_ = findQueueFamiliesWithoutSurface(device);
            indices = indices_;

            // 2. 디바이스 확장 지원 검사
            cBool extensionsSupported = checkDeviceExtensionSupport(device);

            // 3. 물리 디바이스 특성 검사
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            // 4. 디바이스 속성 검사 (옵션)
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            // 스왑체인 검사는 제외하고 기본적인 적합성만 확인
            return indices_.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
        }

        const QueueFamilyIndices findQueueFamiliesWitchSurface(VkPhysicalDevice& device, VkSurfaceKHR& VKsurface)
        {
            QueueFamilyIndices indices; // 큐 패밀리의 개수를 저장할 변수를 초기화
            QueueFamilyIndices target; // 큐 패밀리의 개수를 저장할 변수를 초기화

            // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (첫 번째 호출은 개수만 가져옴)
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (두 번째 호출은 실제 속성을 가져옴)
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef DEBUG_
            getPyhsicalDeviceProperties(device);
#endif // DEBUG_

            int i = 0;
            cBool selected = false;
            for (const auto& queueFamily : queueFamilies) {
                // 현재 큐 패밀리가 그래픽스 큐를 지원하는지 확인
                PRINT_TO_LOGGER("QueueFamily %d\n", i);
                PRINT_TO_LOGGER("QueueFamily queueCount: %d\n", queueFamily.queueCount);

                cString queueFlagsStr;
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    queueFlagsStr += "GRAPHICS ";
                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    queueFlagsStr += "COMPUTE ";
                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    queueFlagsStr += "TRANSFER ";
                if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    queueFlagsStr += "SPARSE_BINDING ";
                if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT)
                    queueFlagsStr += "PROTECTED ";
                if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                    queueFlagsStr += "VIDEO_DECODE ";
                if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                    queueFlagsStr += "VIDEO_ENCODE ";
                if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
                    queueFlagsStr += "OPTICAL_FLOW ";

                PRINT_TO_LOGGER("QueueFamily queueFlags: %s\n", queueFlagsStr.c_str());

                PRINT_TO_LOGGER("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);

                if ((queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
                    && (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)) {
                    PRINT_TO_LOGGER("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    PRINT_TO_LOGGER("VK_QUEUE_COMPUTE_BIT is supported\n");
                    if (!selected)
                    {
                        indices.setgraphicsAndComputeFamily(i);
                    }
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKsurface, &presentSupport);

                if (presentSupport)
                {
                    PRINT_TO_LOGGER("VK_QUEUE_PRESENT_BIT is supported\n");
                    if (!selected)
                    {
                        indices.setPresentFamily(i);
                    }
                }

                if (indices.isComplete()) {

                    if (!selected)
                    {
                        indices.queueFamilyProperties = queueFamily;
                        target = indices;
                        PRINT_TO_LOGGER("------------------ select Queuefamily index: %d ------------------\n", i);
                        selected = true;
                    }
                }

                indices.reset();
                i++;
            }

            return target;
        }

        const QueueFamilyIndices2 findQueueFamiliesWithoutSurface(VkPhysicalDevice& device)
        {
            QueueFamilyIndices2 indices;
            QueueFamilyIndices2 target;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            cBool selected = false;

            for (const auto& queueFamily : queueFamilies) {
                // 그래픽스와 컴퓨트 큐 지원 확인
                PRINT_TO_LOGGER("QueueFamily %d\n", i);
                PRINT_TO_LOGGER("QueueFamily queueCount: %d\n", queueFamily.queueCount);
                PRINT_TO_LOGGER("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
                PRINT_TO_LOGGER("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);


                // 그래픽스 큐 지원 확인
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    PRINT_TO_LOGGER("VK_QUEUE_GRAPHICS_BIT is supported\n");
                    if (!indices.grapicFamilyHasValue) {
                        indices.setGrapicFamily(i);
                    }
                }

                // 컴퓨트 큐 지원 확인  
                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    PRINT_TO_LOGGER("VK_QUEUE_COMPUTE_BIT is supported\n");
                    if (!indices.computerFamilyHasValue) {
                        indices.setComputerFamily(i);
                    }
                }

                // 트랜스퍼 큐 지원 확인
                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    PRINT_TO_LOGGER("VK_QUEUE_TRANSFER_BIT is supported\n");
                    if (!indices.transferFamilyHasValue) {
                        indices.setTransferFamily(i);
                    }
                }

                if (indices.isComplete()) {

                    if (!selected)
                    {
                        indices.queueFamilyProperties = queueFamily;
                        target = indices;
                        selected = true;

                        PRINT_TO_LOGGER("------------------ select Queuefamily index: %d ------------------\n", i);
                    }
                }

                // Surface가 없으므로 Present 큐 확인은 생략
                indices.reset();
                i++;
            }

            return target;
        }

        void copyBuffer(VkDevice VKdevice, VkCommandPool VKcommandPool, VkQueue graphicsVKQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(VKdevice, VKcommandPool);

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(VKdevice, VKcommandPool, graphicsVKQueue, commandBuffer);
        }

        void copyBufferToImage(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { width, height, 1 };

            // buffer의 데이터를 image로 복사한다.
            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void copyBufferToImage2(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::vector<VkDeviceSize>& sizeArray)
        {

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // VKBufferImageCopy 배열을 만든다.
            std::vector<VkBufferImageCopy> bufferCopyRegions;
            VkDeviceSize offset = 0;

            for (uint32_t i = 0; i < sizeArray.size(); i++)
            {
                VkBufferImageCopy region{};

                region.bufferOffset = offset;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = i;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { width, height, 1 };

                bufferCopyRegions.push_back(region);
                offset += sizeArray[i];
            }

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void copyBufferToImage3(VkCommandBuffer cmb, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
        {
            VkBufferImageCopy region{};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = { width, height, 1 };

            // buffer의 데이터를 image로 복사한다.
            vkCmdCopyBufferToImage(cmb, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }

        void copyBufferToImageKTX(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipmapLevels, ktxTexture* textureKTX)
        {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // VKBufferImageCopy 배열을 만든다.
            std::vector<VkBufferImageCopy> bufferCopyRegions;

            for (uint32_t face = 0; face < 6; face++)
            {
                for (uint32_t level = 0; level < mipmapLevels; level++)
                {
                    VkDeviceSize offset = 0;
                    KTX_error_code result = ktxTexture_GetImageOffset(textureKTX, level, 0, face, &offset);
                    _CHECK_RESULT_((result == KTX_SUCCESS));

                    VkBufferImageCopy bufferCopyRegion = {};
                    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    bufferCopyRegion.imageSubresource.mipLevel = level;
                    bufferCopyRegion.imageSubresource.baseArrayLayer = face;
                    bufferCopyRegion.imageSubresource.layerCount = 1;
                    bufferCopyRegion.imageExtent.width = textureKTX->baseWidth >> level;
                    bufferCopyRegion.imageExtent.height = textureKTX->baseHeight >> level;
                    bufferCopyRegion.imageExtent.depth = 1;
                    bufferCopyRegion.bufferOffset = offset;

                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void copyBufferToImageKTX2(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, cUint32_t width, cUint32_t height, cUint32_t mipmapLevels, ktxTexture* textureKTX,cBool useCubmap)
        {
            // VKBufferImageCopy 배열을 만든다.
            std::vector<VkBufferImageCopy> bufferCopyRegions;

            if (useCubmap)
            {
                for (uint32_t face = 0; face < 6; face++)
                {
                    for (uint32_t level = 0; level < mipmapLevels; level++)
                    {
                        VkDeviceSize offset = 0;
                        KTX_error_code result = ktxTexture_GetImageOffset(textureKTX, level, 0, face, &offset);
                        _CHECK_RESULT_((result == KTX_SUCCESS));

                        VkBufferImageCopy bufferCopyRegion = {};
                        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        bufferCopyRegion.imageSubresource.mipLevel = level;
                        bufferCopyRegion.imageSubresource.baseArrayLayer = face;
                        bufferCopyRegion.imageSubresource.layerCount = 1;
                        bufferCopyRegion.imageExtent.width = textureKTX->baseWidth >> level;
                        bufferCopyRegion.imageExtent.height = textureKTX->baseHeight >> level;
                        bufferCopyRegion.imageExtent.depth = 1;
                        bufferCopyRegion.bufferOffset = offset;

                        bufferCopyRegions.push_back(bufferCopyRegion);
                    }
                }
            }
            else
            {
                for (uint32_t level = 0; level < mipmapLevels; level++)
                {
                    VkDeviceSize offset = 0;
                    KTX_error_code result = ktxTexture_GetImageOffset(textureKTX, level, 0, 0, &offset);
                    _CHECK_RESULT_((result == KTX_SUCCESS));

                    VkBufferImageCopy bufferCopyRegion = {};
                    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    bufferCopyRegion.imageSubresource.mipLevel = level;
                    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
                    bufferCopyRegion.imageSubresource.layerCount = 1;
                    bufferCopyRegion.imageExtent.width = textureKTX->baseWidth >> level;
                    bufferCopyRegion.imageExtent.height = textureKTX->baseHeight >> level;
                    bufferCopyRegion.imageExtent.depth = 1;
                    bufferCopyRegion.bufferOffset = offset;

                    bufferCopyRegions.push_back(bufferCopyRegion);
                }
            }

            vkCmdCopyBufferToImage(
                cmd,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());
        }

        cBool checkDeviceExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<cString> requiredExtensions(coreDeviceExtensions.begin(), coreDeviceExtensions.end());

            for (const auto& extension : availableExtensions) {

                PRINT_TO_LOGGER("Available Extension: %s\n", extension.extensionName);
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<cString>* temp)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            for (const auto& extension : availableExtensions) {
                temp->insert(extension.extensionName);
            }
        }

        int rateDeviceSuitability(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            int score = 0;

            // Discrete GPUs have a significant performance advantage
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }

            // Maximum possible size of textures affects graphics quality
            score += deviceProperties.limits.maxImageDimension2D;

            // Application can't function without geometry shaders
            if (!deviceFeatures.geometryShader) {
                return 0;
            }

            PRINT_TO_LOGGER("Device %s score: %d\n", deviceProperties.deviceName, score);
            PRINT_TO_LOGGER("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
            PRINT_TO_LOGGER("Device Name: %s\n", deviceProperties.deviceName);
            PRINT_TO_LOGGER("\n");

            return score;
        }

        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VKsurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
        {
            return findSupportedFormat(
                physicalDevice,
                { VK_FORMAT_D32_SFLOAT,
                  VK_FORMAT_D32_SFLOAT_S8_UINT,
                  VK_FORMAT_D24_UNORM_S8_UINT },    // 후보 형식
                VK_IMAGE_TILING_OPTIMAL,                                                                // 타일링
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT                                          // 특징
            );
        }

        VkFormat findSupportedFormat(
            VkPhysicalDevice physicalDevice,
            const std::vector<VkFormat>& candidates,
            VkImageTiling tiling,
            VkFormatFeatureFlags features)
        {
            VkFormat value{ VK_FORMAT_UNDEFINED };

            for (VkFormat format : candidates)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                {
                    value = format;
                }
                else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                {
                    value = format;
                }
                else
                {
                    PRINT_TO_LOGGER("Failed to find supported format!\n");
                }
            }

            return value;
        }

        uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            return 0;
        }

        void generateMipmaps(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
        {
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

            _CHECK_RESULT_((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT));

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++) {

                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                VkImageBlit blit{};

                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;

                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer,
                    image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);

                if (mipWidth > 1)
                {
                    mipWidth /= 2;
                }

                if (mipHeight > 1)
                {
                    mipHeight /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

        }

        void generateMipmapsCubeMap(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
        {
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

            _CHECK_RESULT_((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT));

            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
            VkImageMemoryBarrier barrier{};

            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1; // Cube map has 6 layers
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; i++)
            {
                for (uint32_t face = 0; face < 6; face++)
                {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.subresourceRange.baseArrayLayer = face;
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

                    VkImageBlit blit{};

                    blit.srcOffsets[0] = { 0, 0, 0 };
                    blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.srcSubresource.mipLevel = i - 1;
                    blit.srcSubresource.baseArrayLayer = face;
                    blit.srcSubresource.layerCount = 1;

                    blit.dstOffsets[0] = { 0, 0, 0 };
                    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    blit.dstSubresource.mipLevel = i;
                    blit.dstSubresource.baseArrayLayer = face;
                    blit.dstSubresource.layerCount = 1;

                    vkCmdBlitImage(commandBuffer,
                        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &blit,
                        VK_FILTER_LINEAR);

                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer,
                        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);
                }

                if (mipWidth > 1)
                {
                    mipWidth /= 2;
                }

                if (mipHeight > 1)
                {
                    mipHeight /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1; // 각 레이어에 대해 개별적으로 전환
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            for (uint32_t i = 0; i < 6; i++) {
                barrier.subresourceRange.baseArrayLayer = i;
                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
            }

            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);

        }
        
        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
        {
            VkCommandBufferAllocateInfo allocInfo{};                          // 커맨드 버퍼 할당 정보 구조체를 초기화합니다.
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // 구조체 타입을 설정합니다.

            // 커맨드 버퍼 레벨을 설정합니다.
            // VK_COMMAND_BUFFER_LEVEL_PRIMARY: 기본 커맨드 버퍼
            // VK_COMMAND_BUFFER_LEVEL_SECONDARY: 보조 커맨드 버퍼
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;                              // 커맨드 풀을 설정합니다.
            allocInfo.commandBufferCount = 1;                                 // 커맨드 버퍼 개수를 설정합니다.

            VkCommandBuffer commandBuffer;
            _VK_CHECK_RESULT_(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));  // 커맨드 버퍼를 할당합니다.

            VkCommandBufferBeginInfo beginInfo{};
            // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO : 명령 버퍼의 시작 정보를 설정합니다.
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 커맨드 버퍼를 한 번만 사용하려는 경우 사용합니다.
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            // 커맨드 버퍼를 시작합니다.
            _VK_CHECK_RESULT_(vkBeginCommandBuffer(commandBuffer, &beginInfo));

            return commandBuffer;
        }

        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue Queue, VkCommandBuffer commandBuffer)
        {
            // 커맨드 버퍼를 종료합니다.
            _VK_CHECK_RESULT_(vkEndCommandBuffer(commandBuffer));

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1; // 커맨드 버퍼 개수를 설정합니다.
            submitInfo.pCommandBuffers = &commandBuffer; // 커맨드 버퍼를 설정합니다.

            // 큐에 커맨드 버퍼를 제출합니다.
            _VK_CHECK_RESULT_(vkQueueSubmit(Queue, 1, &submitInfo, VK_NULL_HANDLE));

            // 큐가 모든 작업을 완료할 때까지 대기합니다.
            _VK_CHECK_RESULT_(vkQueueWaitIdle(Queue));

            // 커맨드 버퍼를 해제합니다.
            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        void transitionImageLayout(
            VkDevice device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            cUint32_t levelCount,
            cUint32_t layerCount)
        {
            // 단일 시간 명령 버퍼를 시작합니다.
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

            // 이미지 메모리 배리어 구조체를 초기화하여 레이아웃 전환 및 접근 권한 변경을 정의합니다.
            // 기본적으로 색상 정보를 대상으로 하지만, 이후 조건에 따라 수정됩니다.
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // 배리어의 구조체 타입 설정
            barrier.oldLayout = oldLayout;                          // 전환 전 이미지 레이아웃
            barrier.newLayout = newLayout;                          // 전환할 이미지 레이아웃
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시
            barrier.image = image; // 전환할 이미지 핸들 설정
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // 첫 번째 미프맵 레벨부터 시작
            barrier.subresourceRange.levelCount = levelCount;// 적용할 미프맵의 수
            barrier.subresourceRange.baseArrayLayer = 0;     // 첫 번째 배열 레이어부터 시작
            barrier.subresourceRange.layerCount = layerCount;         // 배열 내 레이어 개수

            VkPipelineStageFlags sourceStage = 0;      // 전환 전 파이프라인 스테이지
            VkPipelineStageFlags destinationStage = 0; // 전환 후 파이프라인 스테이지

            // 이미지 레이아웃 전환 시, 적절한 액세스 마스크 및 파이프라인 스테이지를 설정합니다.

            switch (oldLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // 이전 레이아웃이 정의되지 않은 상태이면 어떤 액세스도 보장되지 않습니다.
                // or 깊이/스텐실 이미지 초기화 경우로, 이전 레이아웃이 사용 불가능한 상태에서 시작함
                barrier.srcAccessMask = 0;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

            switch (newLayout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // 전송 완료 후 색상 첨부 최적화 레이아웃으로 전환
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // 전송 완료 후 셰이더 읽기 전용으로 전환
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // 전송 쓰기 작업을 위한 쓰기 액세스 허용 설정
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;
            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
                break;
            case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
                break;
            case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
                break;
            case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
                break;
            case VK_IMAGE_LAYOUT_MAX_ENUM:
                break;
            default:
                break;
            }

            // 새로운 레이아웃이 깊이/스텐실용일 경우 aspectMask를 수정합니다.
            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                // 포맷에 스텐실 컴포넌트가 있다면 추가합니다.
                if (hasStencilComponent(format))
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }


            // 전환 타입에 따른 액세스 마스크 및 파이프라인 스테이지 설정
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            }
            else {
                // 지원되지 않는 레이아웃 전환 요청시 예외 발생
                // 1, 모든 파이프라인 스테이지를 사용하여 전환

                sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

                PRINT_TO_LOGGER("All pass pipeline stage!\n");
            }

            // 파이프라인 배리어를 추가하여 레이아웃 전환 명령을 기록합니다.
            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage, // 전환 전/후 스테이지
                0,                // 배리어 플래그 (사용하지 않음)
                0, nullptr,     // 메모리 배리어 없이
                0, nullptr,     // 버퍼 배리어 없이
                1, &barrier     // 하나의 이미지 배리어 사용
            );

            // 단일 명령 버퍼를 제출하고, 큐가 해당 작업을 완료할 때까지 대기합니다.
            endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
        }

        void updateimageLayoutcmd(VkCommandBuffer cmdbuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, cUint32_t levelCount, cUint32_t layerCount)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // 배리어의 구조체 타입 설정
            barrier.oldLayout = oldLayout;                          // 전환 전 이미지 레이아웃
            barrier.newLayout = newLayout;                          // 전환할 이미지 레이아웃
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 소스 큐 패밀리 인덱스 무시
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // 대상 큐 패밀리 인덱스 무시
            barrier.image = image; // 전환할 이미지 핸들 설정
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;       // 첫 번째 미프맵 레벨부터 시작
            barrier.subresourceRange.levelCount = levelCount;// 적용할 미프맵의 수
            barrier.subresourceRange.baseArrayLayer = 0;     // 첫 번째 배열 레이어부터 시작
            barrier.subresourceRange.layerCount = layerCount;         // 배열 내 레이어 개수

            VkPipelineStageFlags sourceStage = 0;      // 전환 전 파이프라인 스테이지
            VkPipelineStageFlags destinationStage = 0; // 전환 후 파이프라인 스테이지

            // 이미지 레이아웃 전환 시, 적절한 액세스 마스크 및 파이프라인 스테이지를 설정합니다.
            barrier.srcAccessMask = getFromOldLayoutToVkAccessFlags(oldLayout);
            barrier.dstAccessMask = getFromNewLayoutToVkAccessFlags(newLayout);

            // 새로운 레이아웃이 깊이/스텐실용일 경우 aspectMask를 수정합니다.
            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                // 포맷에 스텐실 컴포넌트가 있다면 추가합니다.
                if (hasStencilComponent(format))
                {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            }
            else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            // 전환 타입에 따른 액세스 마스크 및 파이프라인 스테이지 설정
            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            }
            else {
                // 지원되지 않는 레이아웃 전환 요청시 예외 발생
                // 1, 모든 파이프라인 스테이지를 사용하여 전환

                sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
                destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

                PRINT_TO_LOGGER("All pass pipeline stage!\n");
            }

            // 파이프라인 배리어를 추가하여 레이아웃 전환 명령을 기록합니다.
            vkCmdPipelineBarrier(
                cmdbuffer,
                sourceStage, destinationStage, // 전환 전/후 스테이지
                0,                // 배리어 플래그 (사용하지 않음)
                0, nullptr,     // 메모리 배리어 없이
                0, nullptr,     // 버퍼 배리어 없이
                1, &barrier     // 하나의 이미지 배리어 사용
            );
        }

    }
}