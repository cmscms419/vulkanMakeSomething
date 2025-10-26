#include "VKshader.h"

using namespace vkengine::Log;

namespace vkengine {

    VKshader::VKshader(VkContext& ctx, cString filepath)
        : ctx(ctx)
    {
        this->name = helper::extractFilename(filepath);

        std::vector<cChar> shaderCode = helper::readSPVFile(filepath);
        
        this->module = helper::createShaderModule(this->ctx.getDevice()->logicaldevice, shaderCode);
        this->reflectModule = helper::createSpvReflectModule(shaderCode);
        this->stage = static_cast<VkShaderStageFlagBits>(this->reflectModule.shader_stage);
    }

    VKshader::VKshader(VKshader&& other) noexcept
        : ctx(other.ctx),
        module(other.module), 
        reflectModule(other.reflectModule), 
        stage(other.stage), 
        name(std::move(other.name))
    {
        other.module = VK_NULL_HANDLE;
        other.reflectModule = {};
        other.stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }

    VKshader::~VKshader()
    {
        cleanup();
    }

    void VKshader::cleanup()
    {
        if (reflectModule._internal != nullptr) {
            spvReflectDestroyShaderModule(&reflectModule);
        }

        if (module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(ctx.getDevice()->logicaldevice, module, nullptr);
            module = VK_NULL_HANDLE;
        }

    }

    std::vector<VkVertexInputAttributeDescription> VKshader::makeVertexInputAttributeDescriptions() const
    {
        std::vector<VkVertexInputAttributeDescription> attributes;

        // 버텍스 쉐이더만 버텍스 입력 속성을 가질 수 있습니다.
        if (this->stage != VK_SHADER_STAGE_VERTEX_BIT) {
            EXIT_TO_LOGGER("Shader stage is not vertex shader. No vertex input attributes.\n");
            return attributes;
        }

        // 입력 변수 개수와 변수 배열이 유효한지 확인
        cUint32_t varCount = this->reflectModule.input_variable_count;
        if (varCount == 0 || this->reflectModule.input_variables == nullptr) {
            PRINT_TO_LOGGER("[Warning] No input variables found in shader: %s\n", name);
            return attributes;
        }

        // verCount의 크기만큼 속성 벡터 예약
        attributes.reserve(varCount);

        // 입력 변수 배열 생성
        std::vector<const SpvReflectInterfaceVariable*> inputVars(this->reflectModule.input_variables,
            this->reflectModule.input_variables + varCount);

        // 각 입력 변수에 대해 VkVertexInputAttributeDescription 생성
        sort(inputVars.begin(), inputVars.end(),
            [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) {
            return a->location < b->location;
        });

        cUint32_t offset = 0;
        for (const SpvReflectInterfaceVariable* var : inputVars) {
            if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
                // 내장 변수는 무시
                continue;
            }

            if (var->location == -1) {
                PRINT_TO_LOGGER("[Warning] Input variable without location in shader: %s\n", name);
                continue;
            }

            if (cString(var->name) == "gl_VertexIndex")
            {
                continue; // gl_VertexIndex는 버텍스 입력 속성이 아니므로 무시
            }

            VkVertexInputAttributeDescription attribute{};
            attribute.location = var->location;
            attribute.binding = 0; // 단일 바인딩 가정
            attribute.format = helper::getVkFormatFromSpvReflectFormat(var->format);
            attribute.offset = offset; // 오프셋은 나중에 버텍스 바인딩에서 설정

            PRINT_TO_LOGGER("Attribute - Location: %d, Binding: %d, Format: %d, Offset: %d, Name: %s\n",
                attribute.location, attribute.binding, attribute.format, attribute.offset,
                var->name ? var->name : "Unknown\n");

            attributes.push_back(attribute);

            offset += helper::getFormatSize(attribute.format);
        }

        return attributes;
    }

}