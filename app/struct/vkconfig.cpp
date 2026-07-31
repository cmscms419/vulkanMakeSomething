#include "vkconfig.h"

// Implementation of factory methods
PipelineConfig PipelineConfig::createGui()
{
    PipelineConfig config;
    config.name = "gui";
    config.requiredFormats.outColorFormat = true;
    config.vertexInput.type = PipelineConfig::VertexInput::Type::ImGui;
    config.colorBlend.blendEnable = true;
    config.specialConfig.hasCustomVertexFormat = true;
    return config;
}
PipelineConfig PipelineConfig::createPbrForward()
{
    PipelineConfig config;
    config.name = "pbrForward";
    config.requiredFormats = {true, true, true}; // color, depth, msaa
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Standard;
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS_OR_EQUAL};
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    config.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    return config;
}

PipelineConfig PipelineConfig::createPbrDeferred()
{
    PipelineConfig config;
    config.name = "pbrdeferred";
    config.requiredFormats = {true, true, true}; // color, depth, msaa
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Standard;
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS_OR_EQUAL};
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    config.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    return config;
}

PipelineConfig PipelineConfig::createPost()
{
    PipelineConfig config;
    config.name = "post";
    config.requiredFormats = {true, true, false}; // color, depth
    config.specialConfig.isScreenSpace = true;
    return config;
}

PipelineConfig PipelineConfig::createShadowMap()
{
    PipelineConfig config;
    config.name = "shadowMap";
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Standard;
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS};
    config.rasterization = {VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, true, true, 1.1f, 2.0f};
    config.dynamicState.states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    config.specialConfig.isDepthOnly = true;
    return config;
}

PipelineConfig PipelineConfig::createSky()
{
    PipelineConfig config;
    config.name = "sky";
    config.requiredFormats = {true, true, true};                      // color, depth, msaa
    config.depthStencil = {true, false, VK_COMPARE_OP_LESS_OR_EQUAL}; // test but no write
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    config.specialConfig.isScreenSpace = true;
    return config;
}

PipelineConfig PipelineConfig::createCompute()
{
    PipelineConfig config;
    config.name = "compute";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createSsao()
{
    PipelineConfig config;
    config.name = "ssao";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createSsaoBlur()
{
    PipelineConfig config;
    config.name = "ssaoBlur";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createDeferredLighting()
{
    PipelineConfig config;
    config.name = "lightdeferred";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createTriangle()
{
    PipelineConfig config;
    config.name = "triangle";
    config.requiredFormats.outColorFormat = true;
    config.specialConfig.isScreenSpace = true;
    return config;
}

PipelineConfig PipelineConfig::createDebugLine()
{
    PipelineConfig config;
    config.name = "debugLine";
    config.requiredFormats = {true, true, true};                      // color, depth, msaa
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Line;
    config.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    config.depthStencil = {true, false, VK_COMPARE_OP_LESS_OR_EQUAL}; // test but no write — 씬을 가리지 않음
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    return config;
}

PipelineConfig PipelineConfig::createInstanced()
{
    PipelineConfig config;
    config.name = "instanced";
    config.requiredFormats = {true, true, true};
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Standard;
    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS_OR_EQUAL}; // pbrdeferred와 동일
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    config.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    return config;
}
