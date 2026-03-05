#ifndef INCLUDE_CONFIG_TYPE_H_
#define INCLUDE_CONFIG_TYPE_H_

#include "common.h"
#include "macros.h"

#include <vector>

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
        ModelConfig character("characters/Leonard/Bboy Hip Hop Move.fbx", "character");
        character.transform = glm::rotate(
            glm::scale(
                glm::translate(
                    glm::mat4(1.0f),
                    glm::vec3(-6.719f, 0.375f, -1.860f)),
                glm::vec3(0.012f)),
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
    std::string name;
    enum class Type
    {
        Graphics,
        Compute
    } type = Type::Graphics;

    // Required format parameters (empty = not required)
    struct RequiredFormats
    {
        bool outColorFormat = false;
        bool depthFormat = false;
        bool msaaSamples = false;
    } requiredFormats;

    // Vertex input configuration
    struct VertexInput
    {
        enum class Type
        {
            None,
            ImGui,
            Standard
        } type = Type::None;
        // None: No vertex input (shader-generated geometry)
        // ImGui: Custom ImGui vertex format
        // Standard: Standard 3D vertex with Vertex::getAttributeDescriptions()
    } vertexInput;

    // Depth/Stencil configuration
    struct DepthStencil
    {
        bool depthTest = false;
        bool depthWrite = false;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    } depthStencil;

    // Rasterization settings (only non-default values)
    struct Rasterization
    {
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        // Shadow-specific settings
        bool depthClampEnable = false;
        bool depthBiasEnable = false;
        float depthBiasConstantFactor = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
    } rasterization;

    // Color blending configuration
    struct ColorBlend
    {
        bool blendEnable = false;

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
        bool isDepthOnly = false;           // ShadowMap: no color attachments
        bool isScreenSpace = false;         // Post/Sky: no vertex buffers
        bool hasCustomVertexFormat = false; // GUI: ImGui vertex format
    } specialConfig;

    static PipelineConfig createGui();
    static PipelineConfig createPbrForward();
    static PipelineConfig createPbrDeferred();
    static PipelineConfig createPost();
    static PipelineConfig createShadowMap();
    static PipelineConfig createSky();
    static PipelineConfig createCompute();
    static PipelineConfig createSsao();
    static PipelineConfig createDeferredLighting();
    static PipelineConfig createTriangle();
};

enum class ResourceAccess : cUint16_t
{
    ColorAttachmentWrite = 0,
    DepthAttachmentWrite,
    ShaderReadOnly,
    ShaderReadWrite, // compute general
    Present,
};

struct ResourceUsage
{
    cString handle; // Like "forwardColor", "shadowDepth", "swapchain"
    ResourceAccess access;
};

#endif // !INCLUDE_CONFIG_TYPE_H_