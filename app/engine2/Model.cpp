#include "Model.h"
#include "ModelNode.h"
#include "ModelLoader.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include "log.h"
#include "macros.h"

namespace vkengine {

namespace
{
    // 로컬 텍스처 인덱스를 전역(TextureTable) 인덱스로 바꾼다.
    // 로컬 쪽은 "텍스처 없음" 을 -1 로 표현한다. Material 캐시 파일 포맷이
    // 그렇게 되어 있어서 바꾸지 않는다 (Material::loadFromCache 참고).
    // 전역 쪽은 -1 을 쓰지 않고 더미 0번(DUMMY_TEXTURE_INDEX)으로 보낸다.
    //
    // 이 함수가 두 규약 사이의 유일한 경계다. 여기를 지나간 뒤로는 -1 이
    // 흘러다니지 않으므로, 셰이더는 항상 유효한 테이블 슬롯을 받는다.
    cInt toGlobalTextureIndex(cInt localIndex, const std::vector<cUint32_t> &localToGlobal)
    {
        if (localIndex < 0 || static_cast<size_t>(localIndex) >= localToGlobal.size())
        {
            return static_cast<cInt>(DUMMY_TEXTURE_INDEX);
        }

        return static_cast<cInt>(localToGlobal[localIndex]);
    }
}

Model::Model(Context& ctx) : ctx(ctx)
{
    rootNode = std::make_unique<ModelNode>();
    rootNode->name = "Root";

    // Initialize animation system - ADD THIS
    animation = std::make_unique<Animation>();
    meshes.emplace_back(this->createMesh());
    globalInverseTransform = cMat4(1.0f);
    boundingBoxMin = cVec3(FLT_MAX);
    boundingBoxMax = cVec3(-FLT_MAX);
    name = "";
    visible = true;
    resource.modelMatrix = cMat4(1.0f);
    resource.materialIndex = 0;
    std::fill(std::begin(resource.coeffs), std::end(resource.coeffs), 0.0f);
}

Model::Model(Model&& other) noexcept
    : ctx(other.ctx), meshes(std::move(other.meshes)), materials(std::move(other.materials)),
      textures(std::move(other.textures)), textureFilenames(std::move(other.textureFilenames)),
      textureSRgb(std::move(other.textureSRgb)), rootNode(std::move(other.rootNode)),
      animation(std::move(other.animation)), name(std::move(other.name)),
      globalInverseTransform(other.globalInverseTransform),
      boundingBoxMin(other.boundingBoxMin), boundingBoxMax(other.boundingBoxMax), visible(other.visible),
      resource(other.resource)
{
    // Reset moved-from object to safe state
    other.globalInverseTransform = cMat4(1.0f);
    other.boundingBoxMin = cVec3(FLT_MAX);
    other.boundingBoxMax = cVec3(-FLT_MAX);
    other.visible = true;
    other.name = "";
    other.resource.modelMatrix = cMat4(1.0f);
    std::fill(std::begin(other.resource.coeffs), std::end(other.resource.coeffs), 0.0f);
}

Model::~Model()
{
    cleanup();
}

void Model::createDescriptorManager2(std::vector<MaterialData> &allMaterials, TextureTable &table)
{
#if 0
    for (size_t i = 0; i < materials.size(); i++) {
        auto& mat = materials[i];
        materialUBO.emplace_back(this->ctx, mat.ubo);
    }

    for (auto& t : textures) {
        t.setSampler(sampler.getSampler());
    }

    materialDescriptorSetHandle.reserve(materials.size());
    materialDescriptorSetHandle.resize(materials.size());

    for (size_t i = 0; i < materials.size(); i++) {
        auto& mat = materials[i];
        auto& b1 = mat.ubo.baseColorTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.baseColorTextureIndex);
        auto& b2 = mat.ubo.emissiveTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.emissiveTextureIndex);
        auto& b3 = mat.ubo.normalTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.normalTextureIndex);
        auto& b4 = mat.ubo.opacityTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.opacityTextureIndex);
        auto& b5 = mat.ubo.metallicRoughnessTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.metallicRoughnessTextureIndex);
        auto& b6 = mat.ubo.occlusionTextureIndex < 0 ? dummyTexture : this->getTexture(mat.ubo.occlusionTextureIndex);
        
        materialDescriptorSetHandle[i].create(ctx, {materialUBO[i].getBuffer(), b1, b2, b3, b4, b5, b6});
    }

    for (auto &ubo : materialUBO)
    {
        ubo.updateData();
    }

#else

    // allMaterials의 사이즈 뒤에서 추가되어야 한다.
    // allMaterials의 뒤에 무엇이 있는지 알 수 없기 때문에, allMaterials.backIndex 뒤 부터 인덱스를 추가해야 한다.
    int materialBaseIndex = int(allMaterials.size());

    // 로컬 인덱스(이 모델 안에서의 번호) -> 전역 인덱스(TextureTable 슬롯 번호) 대응표.
    // 예전에는 "테이블 끝에 순서대로 붙는다" 고 가정하고 textureBaseIndex 를
    // 더했다. 그 가정이 깨지는 순간(예: 테이블이 중복 텍스처를 합치기 시작하면)
    // 조용히 틀린 인덱스가 된다. 테이블이 발급한 번호를 그대로 받아 두면
    // 그런 변경에 영향을 받지 않는다.
    std::vector<cUint32_t> localToGlobal(this->textures.size());
    for (size_t i = 0; i < this->textures.size(); i++)
    {
        localToGlobal[i] = table.add(std::move(this->textures[i]));
    }
    this->textures.clear();

    // 각 Material의 로컬 텍스처 인덱스를 전역 인덱스로 교체.
    if (!materials.empty())
    {
        for (size_t i = 0; i < materials.size(); i++) {
            auto& mat = materials[i];
            mat.ubo.baseColorTextureIndex         = toGlobalTextureIndex(mat.ubo.baseColorTextureIndex, localToGlobal);
            mat.ubo.emissiveTextureIndex          = toGlobalTextureIndex(mat.ubo.emissiveTextureIndex, localToGlobal);
            mat.ubo.normalTextureIndex            = toGlobalTextureIndex(mat.ubo.normalTextureIndex, localToGlobal);
            mat.ubo.opacityTextureIndex           = toGlobalTextureIndex(mat.ubo.opacityTextureIndex, localToGlobal);
            mat.ubo.metallicRoughnessTextureIndex = toGlobalTextureIndex(mat.ubo.metallicRoughnessTextureIndex, localToGlobal);
            mat.ubo.occlusionTextureIndex         = toGlobalTextureIndex(mat.ubo.occlusionTextureIndex, localToGlobal);
        }

        for (const auto& material : materials) {
            allMaterials.push_back(material.ubo);
        }

        for(auto& mesh : this->meshes){
            mesh.materialIndex += materialBaseIndex;
        }
    }
#endif
}

void Model::createVulkanResources()
{
    // Create mesh buffers
    for (auto& mesh : meshes) {
        mesh.createBuffers(ctx);
    }

    // Create material uniform buffers
    // for (auto& material : materials) {
    //    material.createUniformBuffer(ctx);
    //    material.updateUniformBuffer();
    //}
}

void Model::loadFromModelFile(const cString& modelFilename, bool readBistroObj)
{
    ModelLoader modelLoader(*this);
    modelLoader.loadFromModelFile(modelFilename, readBistroObj);
    createVulkanResources();
}

void Model::calculateBoundingBox()
{
    boundingBoxMin = cVec3(FLT_MAX);
    boundingBoxMax = cVec3(-FLT_MAX);

    for (const auto& mesh : meshes) {
        boundingBoxMin = min(boundingBoxMin, mesh.minBounds);
        boundingBoxMax = max(boundingBoxMax, mesh.maxBounds);
    }
}

void Model::cleanup()
{
    for (auto& mesh : meshes) {
        mesh.cleanup(ctx.getDevice()->logicaldevice);
    }

    // for (auto& material : materials) {
    //     material.cleanup(ctx.device());
    // }

    for (auto& texture : textures) {
        texture->cleanup();
    }

    meshes.clear();
    materials.clear();
}

void Model::updateAnimation(float deltaTime)
{
    if (animation && animation->hasAnimations()) {
        animation->updateAnimation(deltaTime);
    }
}

} // namespace vkengine