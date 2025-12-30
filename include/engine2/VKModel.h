#ifndef VK_MODEL_INCLUDE_H_
#define VK_MODEL_INCLUDE_H_

#include "common.h"
#include "log.h"
#include <memory>

#include "VKContext.h"
#include "VKMaterial.h"
#include "VKMesh.h"
#include "VKModelNode.h"
#include "VKSampler.h"
#include "VKImage2D.h"
#include "VKUniformBuffer2.h"
#include "VKAnimation.h"
#include "VKDescriptorSet.h"

namespace vkengine {

class VKModel
{
    friend class ModelLoader;

  public:
    VKModel(VKcontext& ctx);
    VKModel(VKModel&& other) noexcept;
    VKModel(const VKModel&) = delete;
    VKModel& operator=(const VKModel&) = delete;
    ~VKModel();

    void cleanup();
    void createVulkanResources();

    void createDescriptorManager2(VKSampler& sampler, VKImage2D& dummyTexture);

    // VKAnimation methods - ADD THESE
    void updateAnimation(float deltaTime);
    bool hasAnimations() const
    {
        return animation && animation->hasAnimations();
    }
    bool hasBones() const
    {
        return animation && animation->hasBones();
    }
    uint32_t getAnimationCount() const
    {
        return animation ? animation->getAnimationCount() : 0;
    }
    uint32_t getBoneCount() const
    {
        return animation ? animation->getBoneCount() : 0;
    }

    // VKAnimation playback control - ADD THESE
    void playAnimation()
    {
        if (animation)
            animation->getPlay();
    }
    void pauseAnimation()
    {
        if (animation)
            animation->getPause();
    }
    void stopAnimation()
    {
        if (animation)
            animation->getstop();
    }
    bool isAnimationPlaying() const
    {
        return animation && animation->getIsPlaying();
    }
    void setAnimationIndex(uint32_t index)
    {
        if (animation)
            animation->setAnimationIndex(index);
    }
    void setAnimationSpeed(float speed)
    {
        if (animation)
            animation->setPlaybackSpeed(speed);
    }
    void setAnimationLooping(bool loop)
    {
        if (animation)
            animation->setLooping(loop);
    }

    // Bone matrices for shaders - ADD THESE
    const std::vector<cMat4>& getBoneMatrices() const
    {
        static const std::vector<cMat4> empty;
        return animation ? animation->getBoneMatrices() : empty;
    }
    VKAnimation* getAnimation() const
    {
        return animation.get();
    } // Direct access if needed

    std::vector<Mesh>& Meshes()
    {
        return meshes;
    }
    std::vector<VKMaterial>& Materials()
    {
        return materials;
    }
    cUint32_t NumMaterials() const
    {
        return cUint32_t(materials.size());
    }
    VKModelNode* RooNode() const
    {
        return rootNode.get();
    }
    cVec3 BoundingBoxMin() const
    {
        return boundingBoxMin;
    }
    cVec3 BoundingBoxMax() const
    {
        return boundingBoxMax;
    }
    VKImage2D& GetTexture(int index)
    {
        return textures[index];
    }
    std::vector<VKImage2D>& Textures()
    {
        return textures;
    }

    const DescriptorSetHander& MaterialDescriptorSetsManager(uint32_t mat_index)
    {
        return materialDescriptorSetHander[mat_index];
    }

    void loadFromModelFile(const cString& modelFilename, bool readBistroObj);

    auto Name() -> cString&
    {
        return name;
    }

    auto Visible() -> bool&
    {
        return visible;
    }

    auto ModelMatrix() -> cMat4&
    {
        return modelMatrix;
    }

    auto Coeffs() -> float*
    {
        return coeffs;
    }

    Mesh createMesh() {
        return Mesh(ctx);
    }

    void reserveMeshes(size_t count) {
        meshes.reserve(count);
    }

    Mesh& addMesh() {
        meshes.emplace_back(ctx);
        return meshes.back();
    }

  private:
    VKcontext& ctx;

    // VKModel asset data
    std::vector<Mesh> meshes;
    std::vector<VKMaterial> materials;
    std::vector<VKImage2D> textures;
    std::vector<cString> textureFilenames;
    std::vector<cBool> textureSRgb; // sRGB 여부 (임시 저장)
    // 이름이 같은 텍스쳐 중복 생성 방지

    std::unique_ptr<VKModelNode> rootNode;
    std::unique_ptr<VKAnimation> animation;

    cMat4 globalInverseTransform = cMat4(1.0f);

    // Bounding box
    cVec3 boundingBoxMin = cVec3(FLT_MAX);
    cVec3 boundingBoxMax = cVec3(-FLT_MAX);

    std::vector<VKUniformBuffer2<cMaterial2>> materialUBO{};
    std::vector<DescriptorSetHander> materialDescriptorSetHander{};

    cString name{};
    cBool visible = true;
    cMat4 modelMatrix = cMat4(1.0f);
    cFloat coeffs[16] = {0.0f}; // 여러가지 옵션에 사용

    void calculateBoundingBox();
};

}

#endif // !1
