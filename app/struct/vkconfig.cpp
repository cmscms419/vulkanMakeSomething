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

// ---------------------------------------------------------------------------
// 천 시뮬레이션 — 세 패스 모두 compute 이고, set 0 의 바인딩 구성이 동일하다
// (clothCommon.glsl 하나를 세 셰이더가 공유한다). 그래서 디스크립터 세트는
// 하나만 만들어 세 파이프라인에 그대로 바인딩할 수 있다.
// 다른 것은 푸시 상수뿐이고, 그것은 ShaderManager 가 리플렉션으로 알아낸다.
// ---------------------------------------------------------------------------
PipelineConfig PipelineConfig::createCloth()
{
    PipelineConfig config;
    config.name = "cloth";
    config.requiredFormats = {true, true, true};                     // color, depth, msaa
    config.vertexInput.type = PipelineConfig::VertexInput::Type::Cloth;
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS_OR_EQUAL}; // pbrdeferred 와 동일
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    // cullMode 는 기본값 VK_CULL_MODE_NONE 을 그대로 쓴다 — 천은 양면이다.
    return config;
}

PipelineConfig PipelineConfig::createClothPredict()
{
    PipelineConfig config;
    config.name = "clothPredict";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createClothSolve()
{
    PipelineConfig config;
    config.name = "clothSolve";
    config.type = PipelineConfig::Type::Compute;
    return config;
}

PipelineConfig PipelineConfig::createClothFinalize()
{
    PipelineConfig config;
    config.name = "clothFinalize";
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
    config.depthStencil = {true, true, VK_COMPARE_OP_LESS_OR_EQUAL}; // pbrdeferred와 동일
    config.multisample.type = PipelineConfig::Multisample::Type::Variable;
    config.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    return config;
}
