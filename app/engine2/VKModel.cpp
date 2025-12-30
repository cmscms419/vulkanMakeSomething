#include "VKModel.h"
#include "VKModelNode.h"
#include "VKModelLoader.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>

namespace vkengine {

VKModel::VKModel(VKcontext& ctx) : ctx(ctx)
{
    rootNode = std::make_unique<VKModelNode>();
    rootNode->name = "Root";

    // Initialize animation system - ADD THIS
    animation = std::make_unique<VKAnimation>();
    meshes.emplace_back(this->createMesh());
}

VKModel::VKModel(VKModel&& other) noexcept
    : ctx(other.ctx), meshes(std::move(other.meshes)), materials(std::move(other.materials)),
      textures(std::move(other.textures)), textureFilenames(std::move(other.textureFilenames)),
      textureSRgb(std::move(other.textureSRgb)), rootNode(std::move(other.rootNode)),
      animation(std::move(other.animation)), name(std::move(other.name)),
      globalInverseTransform(other.globalInverseTransform),
      boundingBoxMin(other.boundingBoxMin), boundingBoxMax(other.boundingBoxMax),
      materialUBO(std::move(other.materialUBO)),
      materialDescriptorSetHander(std::move(other.materialDescriptorSetHander)), visible(other.visible),
      modelMatrix(other.modelMatrix)
{
    // Reset moved-from object to safe state
    other.globalInverseTransform = cMat4(1.0f);
    other.boundingBoxMin = cVec3(FLT_MAX);
    other.boundingBoxMax = cVec3(-FLT_MAX);
    other.visible = true;
    other.modelMatrix = cMat4(1.0f);
}

VKModel::~VKModel()
{
    cleanup();
}

void VKModel::createDescriptorManager2(VKSampler& sampler, VKImage2D& dummyTexture)
{
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
        auto& b1 = mat.ubo.baseColorTextureIndex < 0
                       ? dummyTexture
                       : this->GetTexture(mat.ubo.baseColorTextureIndex);
        auto& b2 = mat.ubo.emissiveTextureIndex < 0 ? dummyTexture
                                                      : this->GetTexture(mat.ubo.emissiveTextureIndex);
        auto& b3 = mat.ubo.normalTextureIndex < 0 ? dummyTexture
                                                    : this->GetTexture(mat.ubo.normalTextureIndex);
        auto& b4 = mat.ubo.opacityTextureIndex < 0 ? dummyTexture
                                                     : this->GetTexture(mat.ubo.opacityTextureIndex);
        auto& b5 = mat.ubo.metallicRoughnessTextureIndex < 0
                       ? dummyTexture
                       : this->GetTexture(mat.ubo.metallicRoughnessTextureIndex);
        auto& b6 = mat.ubo.occlusionTextureIndex < 0
                       ? dummyTexture
                       : this->GetTexture(mat.ubo.occlusionTextureIndex);
        materialDescriptorSetHander[i].create(ctx, 
                                                 {materialUBO[i].ResourceBinding(),
                                                 b1.ResourceBinding(), b2.ResourceBinding(),
                                                 b3.ResourceBinding(), b4.ResourceBinding(),
                                                 b5.ResourceBinding(), b6.ResourceBinding()});
    }
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
        texture.cleanup();
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