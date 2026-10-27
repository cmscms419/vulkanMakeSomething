#ifndef VK_MODEL_INCLUDE_H_
#define VK_MODEL_INCLUDE_H_

#include "common.h"
#include "materialData.h"
#include "ubo.h"

#include <memory>

#include "Context.h"
#include "Material.h"
#include "Mesh.h"
#include "ModelNode.h"
#include "SamplerHandle.h"
#include "Image2D.h"
#include "UniformBuffer2.h"
#include "Animation.h"
#include "DescriptorSetHandle.h"
#include "TextureTable.h"

namespace vkengine {

class Model
{
    friend class ModelLoader;

  public:
    Model(Context& ctx);
    Model(Model&& other) noexcept;
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    ~Model();

    void cleanup();
    void createVulkanResources();

    void createDescriptorManager2(SamplerHandle& sampler, std::vector<MaterialData>& allMaterials, TextureTable &table);

    // Animation methods - ADD THESE
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

    // Animation playback control - ADD THESE
    void playAnimation()
    {
        if (animation)
            animation->play();
    }
    void pauseAnimation()
    {
        if (animation)
            animation->pause();
    }
    void stopAnimation()
    {
        if (animation)
            animation->stop();
    }
    bool isAnimationPlaying() const
    {
        return animation && animation->isPlaying();
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
    Animation* getAnimation() const
    {
        return animation.get();
    } // Direct access if needed

    std::vector<Mesh>& getMeshes()
    {
        return meshes;
    }
    std::vector<Material>& getMaterials()
    {
        return materials;
    }
    cUint32_t getMaterialCount() const
    {
        return cUint32_t(materials.size());
    }
    ModelNode* getRootNode() const
    {
        return rootNode.get();
    }
    cVec3 getBoundingBoxMin() const
    {
        return boundingBoxMin;
    }
    cVec3 getBoundingBoxMax() const
    {
        return boundingBoxMax;
    }
    std::unique_ptr<Image2D>& getTexture(int index)
    {
        return textures[index];
    }
    std::vector<std::unique_ptr<Image2D>>& getTextures()
    {
        return textures;
    }

    void loadFromModelFile(const cString& modelFilename, bool readBistroObj);

    auto getName() -> cString&
    {
        return name;
    }

    auto isVisible() -> bool&
    {
        return visible;
    }

    DrawModelResource& getModelResource()
    {
        return resource;
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
    Context& ctx;

    // Model asset data
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<std::unique_ptr<Image2D>> textures;
    std::vector<cString> textureFilenames;
    std::vector<cBool> textureSRgb; // sRGB 여부 (임시 저장)
    // 이름이 같은 텍스쳐 중복 생성 방지

    std::unique_ptr<ModelNode> rootNode;
    std::unique_ptr<Animation> animation;

    cMat4 globalInverseTransform;

    // Bounding box
    cVec3 boundingBoxMin;
    cVec3 boundingBoxMax;

    cString name;
    cBool visible;
    DrawModelResource resource;

    void calculateBoundingBox();
};

}

#endif // !1
