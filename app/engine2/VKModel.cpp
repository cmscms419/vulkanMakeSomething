#include "VKModel.h"
#include "VKModelNode.h"
#include "VKModelLoader.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>
#include "log.h"

namespace vkengine {

VKModel::VKModel(VKcontext& ctx) : ctx(ctx)
{
    rootNode = std::make_unique<VKModelNode>();
    rootNode->name = "Root";

    // Initialize animation system - ADD THIS
    animation = std::make_unique<VKAnimation>();
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

VKModel::VKModel(VKModel&& other) noexcept
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

VKModel::~VKModel()
{
    cleanup();
}

void VKModel::createDescriptorManager2(VKSamplerHandler &sampler, std::vector<cMaterial> &allMaterials, VKtexturesTable &table)
{
#if 0
    for (size_t i = 0; i < materials.size(); i++) {
        auto& mat = materials[i];
        materialUBO.emplace_back(this->ctx, mat.ubo);
    }

    for (auto& t : textures) {
        t.setSampler(sampler.getSampler());
    }

    materialDescriptorSetHander.reserve(materials.size());
    materialDescriptorSetHander.resize(materials.size());

    for (size_t i = 0; i < materials.size(); i++) {
        auto& mat = materials[i];
        auto& b1 = mat.ubo.baseColorTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.baseColorTextureIndex);
        auto& b2 = mat.ubo.emissiveTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.emissiveTextureIndex);
        auto& b3 = mat.ubo.normalTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.normalTextureIndex);
        auto& b4 = mat.ubo.opacityTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.opacityTextureIndex);
        auto& b5 = mat.ubo.metallicRoughnessTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.metallicRoughnessTextureIndex);
        auto& b6 = mat.ubo.occlusionTextureIndex < 0 ? dummyTexture : this->GetTexture(mat.ubo.occlusionTextureIndex);
        
        materialDescriptorSetHander[i].create(ctx, {materialUBO[i].Buffer(), b1, b2, b3, b4, b5, b6});
    }

    for (auto &ubo : materialUBO)
    {
        ubo.updateData();
    }

#else
    for (auto& t : textures) {
        t->setSampler(sampler.getSampler());
    }

    // allMaterials의 사이즈 뒤에서 추가되어야 한다.
    // allMaterials의 뒤에 무엇이 있는지 알 수 없기 때문에, allMaterials.backIndex 뒤 부터 인덱스를 추가해야 한다.
    // texture 또한 마찬가지 이다.
    int materialBaseIndex = int(allMaterials.size());
    int textureBaseIndex = int(table.getTextures().size());

    // texture의 객수 만큼 범위를 할당     
    table.getTextures().reserve(textureBaseIndex + this->textures.size());
    for (auto& texture : this->textures)
    {
        table.getTextures().push_back(std::move(texture));
    }
    this->textures.clear();
    
    // 각 Material의 로컬 텍스처 인덱스를 전역 인덱스로 교체
    if (!materials.empty())
    {
        for (size_t i = 0; i < materials.size(); i++) {
            auto& mat = materials[i];
            if (mat.ubo.baseColorTextureIndex != -1) {
                mat.ubo.baseColorTextureIndex += textureBaseIndex;
            }
            if (mat.ubo.emissiveTextureIndex != -1) {
                mat.ubo.emissiveTextureIndex += textureBaseIndex;
            }
            if (mat.ubo.normalTextureIndex != -1) {
                mat.ubo.normalTextureIndex += textureBaseIndex;
            }
            if (mat.ubo.opacityTextureIndex != -1) {
                mat.ubo.opacityTextureIndex += textureBaseIndex;
            }
            if (mat.ubo.metallicRoughnessTextureIndex != -1) {
                mat.ubo.metallicRoughnessTextureIndex += textureBaseIndex;
            }
            if (mat.ubo.occlusionTextureIndex != -1) {
                mat.ubo.occlusionTextureIndex += textureBaseIndex;
            }

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

void VKModel::createVulkanResources()
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

void VKModel::loadFromModelFile(const cString& modelFilename, bool readBistroObj)
{
    ModelLoader modelLoader(*this);
    modelLoader.loadFromModelFile(modelFilename, readBistroObj);
    createVulkanResources();
}

void VKModel::calculateBoundingBox()
{
    boundingBoxMin = cVec3(FLT_MAX);
    boundingBoxMax = cVec3(-FLT_MAX);

    for (const auto& mesh : meshes) {
        boundingBoxMin = min(boundingBoxMin, mesh.minBounds);
        boundingBoxMax = max(boundingBoxMax, mesh.maxBounds);
    }
}

void VKModel::cleanup()
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

void VKModel::updateAnimation(float deltaTime)
{
    if (animation && animation->hasAnimations()) {
        animation->updateAnimation(deltaTime);
    }
}

} // namespace vkengine