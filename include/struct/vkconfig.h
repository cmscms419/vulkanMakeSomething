#ifndef INCLUDE_CONFIG_TYPE_H_
#define INCLUDE_CONFIG_TYPE_H_

#include "common.h"
#include "macros.h"

#include <vector>
#include <string>
#include <unordered_map>

// NEW: Model configuration structure
struct ModelConfig
{
    cString filePath;              // Relative to assets path
    cString displayName;           // Display name for GUI
    cMat4 transform = cMat4(1.0f); // Model transformation matrix
    cBool isBistroObj = false;     // Special handling for Bistro models

    // Animation settings
    cBool autoPlayAnimation = true;      // Start animation automatically
    cUint32_t initialAnimationIndex = 0; // Which animation to start with
    cFloat animationSpeed = 1.0f;        // Animation playback speed
    cBool loopAnimation = true;          // Loop the animation

    // Helper constructors
    ModelConfig() = default;

    ModelConfig(const cString &path, const cString &name = "",
                const cMat4 &trans = cMat4(1.0f), cBool bistro = false)
        : filePath(path), displayName(name.empty() ? path : name), transform(trans),
          isBistroObj(bistro)
    {
    }

    // Fluent interface for easy configuration
    ModelConfig &setScale(cFloat scale)
    {
        transform = glm::scale(transform, cVec3(scale));
        return *this;
    }

    ModelConfig &setName(const cString &name)
    {
        displayName = name;
        return *this;
    }
    ModelConfig &setTransform(const cMat4 &trans)
    {
        transform = trans;
        return *this;
    }
    ModelConfig &setBistroModel(cBool bistro)
    {
        isBistroObj = bistro;
        return *this;
    }
    ModelConfig &setAnimation(cBool autoPlay, cUint32_t index = 0, cFloat speed = 1.0f,
                              cBool loop = true)
    {
        autoPlayAnimation = autoPlay;
        initialAnimationIndex = index;
        animationSpeed = speed;
        loopAnimation = loop;
        return *this;
    }
};

// NEW: Application configuration structure
struct ApplicationConfig
{
    std::vector<ModelConfig> models;

    static ApplicationConfig createDefault()
    {
        ApplicationConfig config;

        // Character model
        ModelConfig character("characters/demoModel.fbx", "character");
        character.transform = glm::rotate(
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-6.719f, 0.0f, -1.860f)), glm::vec3(0.012f)),
            glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        character.autoPlayAnimation = true;

        // Bistro scene
        ModelConfig bistro("AmazonLumberyardBistroMorganMcGuire/exterior.obj", "world",
                           glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)),
                           true // isBistroObj
        );
        bistro.autoPlayAnimation = false;

        config.models = {character, bistro};

        return config;
    }
};

// Core pipeline configuration structure
struct PipelineConfig
{
    // Pipeline type and identification
    cString name;
    enum class Type
    {
        Graphics,
        Compute
    } type = Type::Graphics;

    // Required format parameters (empty = not required)
    struct RequiredFormats
    {
        cBool outColorFormat = false;
        cBool depthFormat = false;
        cBool msaaSamples = false;
    } requiredFormats;

    // Vertex input configuration
    struct VertexInput
    {
        enum class Type
        {
            None,
            ImGui,
            Standard,
            Line,
            InstanceData
        } type = Type::None;
        // None: No vertex input (shader-generated geometry)
        // ImGui: Custom ImGui vertex format
        // Standard: Standard 3D vertex with Vertex::getAttributeDescriptions()
        // Line: LineVertex (pos+color) — debug line rendering
        // InstanceData: InstanceData (TRS matrix) — instanced rendering
    } vertexInput;

    // Primitive topology (default TRIANGLE_LIST — 대부분의 파이프라인이 이 값 사용)
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Depth/Stencil configuration
    struct DepthStencil
    {
        cBool depthTest = false;
        cBool depthWrite = false;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    } depthStencil;

    // Rasterization settings (only non-default values)
    struct Rasterization
    {
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        // Shadow-specific settings
        cBool depthClampEnable = false;
        cBool depthBiasEnable = false;
        cFloat depthBiasConstantFactor = 0.0f;
        cFloat depthBiasSlopeFactor = 0.0f;
    } rasterization;

    // Color blending configuration
    struct ColorBlend
    {
        cBool blendEnable = false;

        // Alpha blending preset (for GUI)
        struct AlphaBlending
        {
            VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        } alphaBlending;
    } colorBlend;

    // Multisampling configuration
    struct Multisample
    {
        enum class Type
        {
            Single,
            Variable
        } type = Type::Single;
        // Single: VK_SAMPLE_COUNT_1_BIT
        // Variable: Uses msaaSamples parameter
    } multisample;

    // Dynamic state configuration
    struct DynamicState
    {
        std::vector<VkDynamicState> states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        // Shadow maps add VK_DYNAMIC_STATE_DEPTH_BIAS
    } dynamicState;

    // Pipeline-specific configurations
    struct SpecialConfig
    {
        cBool isDepthOnly = false;           // ShadowMap: no color attachments
        cBool isScreenSpace = false;         // Post/Sky: no vertex buffers
        cBool hasCustomVertexFormat = false; // GUI: ImGui vertex format
    } specialConfig;

    static PipelineConfig createGui();
    static PipelineConfig createPbrForward();
    static PipelineConfig createPbrDeferred();
    static PipelineConfig createPost();
    static PipelineConfig createShadowMap();
    static PipelineConfig createSky();
    static PipelineConfig createCompute();
    static PipelineConfig createSsao();
    static PipelineConfig createSsaoBlur();
    static PipelineConfig createDeferredLighting();
    static PipelineConfig createTriangle();
    static PipelineConfig createDebugLine();
    static PipelineConfig createInstanced();
};

enum class ResourceAccess : cUint16_t
{
    NOTTHING = 0,
    ColorAttachmentWrite,
    DepthAttachmentWrite,
    ShaderReadOnly,
    ShaderWriteOnly,
    ShaderReadWrite, // compute general
    Present,
    MAX
};

struct ResourceUsage
{
    cString handle; // Like "forwardColor", "shadowDepth", "swapchain"
    ResourceAccess access;
};

static inline cString getStringResourceAccess(ResourceAccess access)
{
    switch (access)
    {
    case ResourceAccess::NOTTHING:
        return "NOTTHING";
    case ResourceAccess::ColorAttachmentWrite:
        return "ColorAttachmentWrite";
    case ResourceAccess::DepthAttachmentWrite:
        return "DepthAttachmentWrite";
    case ResourceAccess::ShaderReadOnly:
        return "ShaderReadOnly";
    case ResourceAccess::ShaderWriteOnly:
        return "ShaderWriteOnly";
    case ResourceAccess::ShaderReadWrite:
        return "ShaderReadWrite";
    case ResourceAccess::Present:
        return "Present";
    default:
        return "Unknown";
    }
}

static inline ResourceAccess getResourceAccess(cString access)
{
    ResourceAccess result = ResourceAccess::NOTTHING;

    if (access == "ColorAttachmentWrite")
    {
        result = ResourceAccess::ColorAttachmentWrite;
    }
    else if (access == "DepthAttachmentWrite")
    {
        result = ResourceAccess::DepthAttachmentWrite;
    }
    else if (access == "ShaderReadOnly")
    {
        result = ResourceAccess::ShaderReadOnly;
    }
    else if (access == "ShaderWriteOnly")
    {
        result = ResourceAccess::ShaderWriteOnly;
    }
    else if (access == "ShaderReadWrite")
    {
        result = ResourceAccess::ShaderReadWrite;
    }
    else if (access == "Present")
    {
        result = ResourceAccess::Present;
    }
    else
    {
        result = ResourceAccess::NOTTHING;
    }

    return result;
}

#endif // !INCLUDE_CONFIG_TYPE_H_