#ifndef _VK_ANIMATION_INCLUDE_H_
#define _VK_ANIMATION_INCLUDE_H_

#include "common.h"

#include <cString>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include <assimp/anim.h>

namespace vkengine {

template <typename T>
struct AnimationKey
{
    double time; // Time in animation (usually in seconds)
    T value;     // Value at this keyframe

    AnimationKey() : time(0.0)
    {
    }
    AnimationKey(double t, const T& v) : time(t), value(v)
    {
    }
};

using PositionKey = AnimationKey<cVec3>;
using RotationKey = AnimationKey<cQuat>;
using ScaleKey = AnimationKey<cVec3>;

struct AnimationChannel
{
    cString nodeName; // Name of the target node/bone

    std::vector<PositionKey> positionKeys; // Position keyframes
    std::vector<RotationKey> rotationKeys; // Rotation keyframes
    std::vector<ScaleKey> scaleKeys;       // Scale keyframes

    // Interpolation methods for keyframes
    cVec3 interpolatePosition(double time) const;
    cQuat interpolateRotation(double time) const;
    cVec3 interpolateScale(double time) const;

  private:
    template <typename T>
    T interpolateKeys(const std::vector<AnimationKey<T>>& keys, double time) const;
};

struct Bone
{
    cString name;              // Bone name
    int id;                   // Unique bone ID
    cMat4 offsetMatrix;        // Inverse bind pose matrix
    cMat4 finalTransformation; // Final transformation matrix
    int parentIndex;          // Parent bone index

    // Vertex weights influenced by this bone
    struct VertexWeight
    {
        cUint32_t vertexId;
        float weight;
    };
    std::vector<VertexWeight> weights;

    Bone() : id(-1), offsetMatrix(1.0f), finalTransformation(1.0f), parentIndex(-1)
    {
    }
};

class VKAnimation
{
  public:
    VKAnimation();
    ~VKAnimation();

    // VKAnimation loading and processing
    void loadFromScene(const aiScene* scene);
    void processAnimations(const aiScene* scene);
    void processBones(const aiScene* scene);
    void buildSceneGraph(const aiScene* scene);

    void buildBoneHierarchy(const aiScene* scene);
    void assignGlobalBoneIds();

    void updateAnimation(float timeInSeconds);
    void setAnimationIndex(cUint32_t index);
    void setPlaybackSpeed(float speed);
    void setLooping(bool loop);

    void calculateBoneTransforms(std::vector<cMat4>& transforms, const cString& nodeName = "",
                                 const cMat4& parentTransform = cMat4(1.0f));
    cMat4 getNodeTransformation(const cString& nodeName, double time) const;

    int getGlobalBoneIndex(const cString& boneName) const;

    // Getters
    bool hasAnimations() const
    {
        return !animations_.empty();
    }
    bool hasBones() const
    {
        return !bones.empty();
    }
    cUint32_t getAnimationCount() const
    {
        return static_cast<cUint32_t>(animations_.size());
    }
    cUint32_t getBoneCount() const
    {
        return static_cast<cUint32_t>(bones.size());
    }
    float getDuration() const;
    float getCurrentTime() const
    {
        return currentTime;
    }
    const cString& getCurrentAnimationName() const;

    // Bone matrix access for shaders
    const std::vector<cMat4>& getBoneMatrices() const
    {
        return boneMatrices;
    }
    const cMat4& getGlobalInverseTransform() const
    {
        return globalInverseTransform;
    }

    // VKAnimation state
    const cBool getIsPlaying()
    {
        return this->isPlaying;
    }
    void getPlay()
    {
        this->isPlaying = true;
    }
    void getPause()
    {
        this->isPlaying = false;
    }
    void getstop()
    {
        this->isPlaying = false;
        this->currentTime = 0.0f;
    }

    void setGlobalInverseTransform(const cMat4& transform)
    {
        globalInverseTransform = transform;
    }

  private:
    struct SceneNode;

    struct AnimationClip
    {
        cString name;
        double duration;       // In seconds
        double ticksPerSecond; // VKAnimation speed
        std::vector<AnimationChannel> channels;

        AnimationClip() : duration(0.0), ticksPerSecond(25.0)
        {
        }
    };

    struct SceneNode
    {
        cString name;
        cMat4 transformation;
        std::vector<std::unique_ptr<SceneNode>> children;
        SceneNode* parent = nullptr;

        SceneNode(const cString& n) : name(n), transformation(1.0f)
        {
        }
    };

    std::vector<AnimationClip> animations_;
    std::vector<Bone> bones;
    std::unordered_map<cString, cUint32_t> boneMapping_; // Bone name to index mapping

    std::unordered_map<cString, int> globalBoneNameToId; // Global bone name to ID mapping
    std::vector<cString> globalBoneIdToName_;             // Global bone ID to name mapping

    // Playback state
    cUint32_t currentAnimationIndex;
    float currentTime;
    float playbackSpeed;
    cBool isPlaying;
    cBool isLooping;

    std::vector<cMat4> boneMatrices;   // Final bone transformation matrices
    cMat4 globalInverseTransform; // Global inverse transformation

    // Scene graph data
    std::unique_ptr<SceneNode> rootNode;
    std::unordered_map<cString, SceneNode*> nodeMapping; // Quick node lookup

    // Helper methods
    void processAnimationChannel(const aiNodeAnim* nodeAnim, AnimationChannel& channel);
    cVec3 convertVector(const aiVector3D& vec) const;
    cQuat convertQuaternion(const aiQuaternion& cQuat) const;

    // Keyframe extraction helpers
    void extractPositionKeys(const aiNodeAnim* nodeAnim, std::vector<PositionKey>& keys);
    void extractRotationKeys(const aiNodeAnim* nodeAnim, std::vector<RotationKey>& keys);
    void extractScaleKeys(const aiNodeAnim* nodeAnim, std::vector<ScaleKey>& keys);

    // Helper methods for scene graph
    std::unique_ptr<SceneNode> buildSceneNode(const aiNode* aiNode, SceneNode* parent);
    void traverseNodeHierarchy(SceneNode* node, const cMat4& parentTransform,
        std::vector<cMat4>& boneTransforms);
};

} // namespace vkengine

#endif // _VK_ANIMATION_INCLUDE_H_