#include "VKAnimation.h"
#include "log.h"
#include <algorithm>
#include <functional>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtc/type_ptr.hpp> 
// glm::make_mat4를 사용하기 위해서 다시 선언한다.

using namespace vkengine::Log;

namespace vkengine {

VKAnimation::VKAnimation()
    : currentAnimationIndex(0), currentTime(0.0f), playbackSpeed(1.0f), isPlaying(false),
      isLooping(true), globalInverseTransform(1.0f)
{
}

VKAnimation::~VKAnimation() = default;

void VKAnimation::loadFromScene(const aiScene* scene)
{
    if (!scene) {
        PRINT_TO_LOGGER("VKAnimation::loadFromScene - Invalid scene");
        return;
    }

    PRINT_TO_LOGGER("Loading animation data from scene...\n");
    PRINT_TO_LOGGER("  Animations found: %d", scene->mNumAnimations);

    // Store global inverse transform
    if (scene->mRootNode) {
        globalInverseTransform =
            glm::inverse(glm::transpose(glm::make_mat4(&scene->mRootNode->mTransformation.a1)));
    }

    buildSceneGraph(scene);
    processBones(scene);
    buildBoneHierarchy(scene);
    assignGlobalBoneIds();

    if (scene->mNumAnimations > 0) {
        processAnimations(scene);
    }

    // Initialize bone matrices
    boneMatrices.resize(bones.size(), cMat4(1.0f));

    PRINT_TO_LOGGER("VKAnimation loading complete:");
    PRINT_TO_LOGGER("  VKAnimation clips: %d", animations_.size());
    PRINT_TO_LOGGER("  Bones: %d", bones.size());
    PRINT_TO_LOGGER("  Scene nodes: %d", nodeMapping.size());
}

void VKAnimation::processBones(const aiScene* scene)
{
    if (!scene)
        return;

    PRINT_TO_LOGGER("Processing bones for global hierarchy...");

    // Collect all unique bone names from all meshes
    std::unordered_map<cString, cMat4> boneOffsetMatrices;
    std::unordered_map<cString, std::vector<Bone::VertexWeight>> boneWeights;

    uint32_t totalMeshBones = 0;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasBones())
            continue;

        // PRINT_TO_LOGGER("  Processing {} bones from mesh '{}'", mesh->mNumBones, mesh->mName.C_Str());

        totalMeshBones += mesh->mNumBones;

        for (uint32_t boneIdx = 0; boneIdx < mesh->mNumBones; ++boneIdx) {
            const aiBone* aiBone = mesh->mBones[boneIdx];
            cString boneName = aiBone->mName.C_Str();

            // Store offset matrix (should be same for all meshes)
            if (boneOffsetMatrices.find(boneName) == boneOffsetMatrices.end()) {
                boneOffsetMatrices[boneName] =
                    glm::transpose(glm::make_mat4(&aiBone->mOffsetMatrix.a1));
            }

            // Collect vertex weights
            for (uint32_t weightIdx = 0; weightIdx < aiBone->mNumWeights; ++weightIdx) {
                const aiVertexWeight& weight = aiBone->mWeights[weightIdx];
                boneWeights[boneName].push_back({weight.mVertexId, weight.mWeight});
            }
        }
    }

    // Create global bone list with proper IDs
    bones.clear();
    boneMapping_.clear();
    globalBoneNameToId.clear();

    uint32_t globalBoneIndex = 0;
    for (const auto& pair : boneOffsetMatrices) {
        const cString& boneName = pair.first;
        const cMat4& offsetMatrix = pair.second;

        Bone bone;
        bone.name = boneName;
        bone.id = globalBoneIndex;
        bone.offsetMatrix = offsetMatrix;
        bone.weights = boneWeights[boneName];

        bones.push_back(bone);
        boneMapping_[boneName] = globalBoneIndex;
        globalBoneNameToId[boneName] = globalBoneIndex;

        globalBoneIndex++;
    }

    PRINT_TO_LOGGER("Created %d global bones from %d total mesh bones\n", bones.size(), totalMeshBones);

    // After collecting weights, verify they sum to 1.0
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (!mesh->HasBones())
            continue;

        // Track total weight per vertex
        std::vector<float> vertexWeightSums(mesh->mNumVertices, 0.0f);

        for (uint32_t boneIdx = 0; boneIdx < mesh->mNumBones; ++boneIdx) {
            const aiBone* aiBone = mesh->mBones[boneIdx];
            for (uint32_t weightIdx = 0; weightIdx < aiBone->mNumWeights; ++weightIdx) {
                const aiVertexWeight& weight = aiBone->mWeights[weightIdx];
                vertexWeightSums[weight.mVertexId] += weight.mWeight;
            }
        }

        // Verify weights sum to 1.0 (with epsilon tolerance)
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
            if (vertexWeightSums[i] > 0.0f && std::abs(vertexWeightSums[i] - 1.0f) > 0.01f) {
                PRINT_TO_LOGGER("WARNING: Vertex {} in mesh '{}' has total weight {:.3f} (expected 1.0)\n",
                         i, mesh->mName.C_Str(), vertexWeightSums[i]);
            }
        }
    }
}

void VKAnimation::buildBoneHierarchy(const aiScene* scene)
{
    if (!scene || !scene->mRootNode)
        return;

    PRINT_TO_LOGGER("Building bone hierarchy...\n");

    // Reset parent indices
    for (auto& bone : bones) {
        bone.parentIndex = -1;
    }

    // Function to find parent bone recursively
    std::function<const aiNode*(const aiNode*)> findBoneParent =
        [&](const aiNode* node) -> const aiNode* {
        if (!node || !node->mParent)
            return nullptr;

        if (globalBoneNameToId.find(node->mParent->mName.C_Str()) != globalBoneNameToId.end()) {
            return node->mParent;
        }

        return findBoneParent(node->mParent);
    };

    // Function to traverse scene nodes and establish hierarchy
    std::function<void(const aiNode*)> traverseNodes = [&](const aiNode* node) {
        if (!node)
            return;

        cString nodeName = node->mName.C_Str();

        // If this node represents a bone
        if (globalBoneNameToId.find(nodeName) != globalBoneNameToId.end()) {
            int boneIndex = globalBoneNameToId[nodeName];

            // Find parent bone
            const aiNode* parentBone = findBoneParent(node);
            if (parentBone) {
                cString parentName = parentBone->mName.C_Str();
                if (globalBoneNameToId.find(parentName) != globalBoneNameToId.end()) {
                    int parentIndex = globalBoneNameToId[parentName];
                    bones[boneIndex].parentIndex = parentIndex;
                    PRINT_TO_LOGGER("  Bone '%s' [%d] -> parent '%s' [%d]\n", nodeName, boneIndex,
                             parentName, parentIndex);
                }
            }
        }

        // Process children
        for (uint32_t i = 0; i < node->mNumChildren; ++i) {
            traverseNodes(node->mChildren[i]);
        }
    };

    traverseNodes(scene->mRootNode);
    PRINT_TO_LOGGER("Bone hierarchy established\n");
}

void VKAnimation::assignGlobalBoneIds()
{
    // Create ID to name mapping
    globalBoneIdToName_.resize(bones.size());
    for (const auto& pair : globalBoneNameToId) {
        globalBoneIdToName_[pair.second] = pair.first;
    }

    PRINT_TO_LOGGER("Global bone ID assignment complete: {} bones", bones.size());
}

int VKAnimation::getGlobalBoneIndex(const cString& boneName) const
{
    auto it = globalBoneNameToId.find(boneName);
    return (it != globalBoneNameToId.end()) ? it->second : -1;
}

void VKAnimation::processAnimations(const aiScene* scene)
{
    animations_.reserve(scene->mNumAnimations);

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* aiAnim = scene->mAnimations[i];

        AnimationClip clip;
        clip.name = aiAnim->mName.C_Str();
        clip.duration = aiAnim->mDuration;
        clip.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0;

        // PRINT_TO_LOGGER("Processing animation '{}' - Duration: {:.2f}s, FPS: {:.1f}", clip.name,
        //          clip.duration / clip.ticksPerSecond, clip.ticksPerSecond);

        // Process animation channels
        clip.channels.reserve(aiAnim->mNumChannels);
        for (uint32_t j = 0; j < aiAnim->mNumChannels; ++j) {
            const aiNodeAnim* nodeAnim = aiAnim->mChannels[j];

            AnimationChannel channel;
            processAnimationChannel(nodeAnim, channel);
            clip.channels.push_back(std::move(channel));
        }

        animations_.push_back(std::move(clip));
    }
}

void VKAnimation::processAnimationChannel(const aiNodeAnim* nodeAnim, AnimationChannel& channel)
{
    channel.nodeName = nodeAnim->mNodeName.C_Str();

    // Extract position keys
    extractPositionKeys(nodeAnim, channel.positionKeys);

    // Extract rotation keys
    extractRotationKeys(nodeAnim, channel.rotationKeys);

    // Extract scale keys
    extractScaleKeys(nodeAnim, channel.scaleKeys);

    // PRINT_TO_LOGGER("  Channel '{}': {} pos, {} rot, {} scale keys", channel.nodeName,
    //          channel.positionKeys.size(), channel.rotationKeys.size(), channel.scaleKeys.size());
}

void VKAnimation::extractPositionKeys(const aiNodeAnim* nodeAnim, std::vector<PositionKey>& keys)
{
    keys.reserve(nodeAnim->mNumPositionKeys);
    for (uint32_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
        const aiVectorKey& key = nodeAnim->mPositionKeys[i];
        keys.emplace_back(key.mTime, cVec3(key.mValue.x, key.mValue.y, key.mValue.z));
    }
}

void VKAnimation::extractRotationKeys(const aiNodeAnim* nodeAnim, std::vector<RotationKey>& keys)
{
    keys.reserve(nodeAnim->mNumRotationKeys);
    for (uint32_t i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
        const aiQuatKey& key = nodeAnim->mRotationKeys[i];
        keys.emplace_back(key.mTime, cQuat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
    }
}

void VKAnimation::extractScaleKeys(const aiNodeAnim* nodeAnim, std::vector<ScaleKey>& keys)
{
    keys.reserve(nodeAnim->mNumScalingKeys);
    for (uint32_t i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
        const aiVectorKey& key = nodeAnim->mScalingKeys[i];
        keys.emplace_back(key.mTime, cVec3(key.mValue.x, key.mValue.y, key.mValue.z));
    }
}

void VKAnimation::updateAnimation(float deltaTime)
{
    if (!isPlaying || animations_.empty())
        return;

    currentTime += deltaTime * playbackSpeed;

    const AnimationClip& currentAnim = animations_[currentAnimationIndex];
    double animationTime = currentTime * currentAnim.ticksPerSecond;

    // Handle looping
    if (isLooping && animationTime > currentAnim.duration) {
        currentTime = 0.0f;
        animationTime = 0.0;
    } else if (!isLooping && animationTime > currentAnim.duration) {
        currentTime = static_cast<float>(currentAnim.duration / currentAnim.ticksPerSecond);
        animationTime = currentAnim.duration;
        isPlaying = false;
    }

    // Update bone transformations using proper hierarchy
    if (rootNode) {
        calculateBoneTransforms(boneMatrices);
    }
}

void VKAnimation::calculateBoneTransforms(std::vector<cMat4>& transforms, const cString& nodeName,
                                        const cMat4& parentTransform)
{
    if (animations_.empty() || !rootNode) {
        return;
    }

    // Start hierarchical traversal from root
    traverseNodeHierarchy(rootNode.get(), cMat4(1.0f), transforms);
}

void VKAnimation::traverseNodeHierarchy(SceneNode* node, const cMat4& parentTransform,
                                      std::vector<cMat4>& boneTransforms)
{
    if (!node)
        return;

    const AnimationClip& currentAnim = animations_[currentAnimationIndex];
    double animationTime = currentTime * currentAnim.ticksPerSecond;

    // Get animated transformation for this node
    cMat4 nodeTransformation = getNodeTransformation(node->name, animationTime);

    // If no animation channel exists, use the original transformation
    if (nodeTransformation == cMat4(1.0f)) {
        nodeTransformation = node->transformation;
    }

    // Calculate global transformation
    cMat4 globalTransformation = parentTransform * nodeTransformation;

    // Check if this node is a bone
    auto it = globalBoneNameToId.find(node->name);
    if (it != globalBoneNameToId.end()) {
        uint32_t boneIndex = it->second;
        if (boneIndex < bones.size() && boneIndex < boneTransforms.size()) {
            // FIXED: Correct bone transformation calculation
            // The proper formula is: FinalTransform = GlobalInverse * GlobalTransform *
            // OffsetMatrix This transforms: vertex -> bone space (offset) -> world space (global)
            // -> root space (inverse)
            boneTransforms[boneIndex] =
                globalInverseTransform * globalTransformation * bones[boneIndex].offsetMatrix;
        }
    }

    // Recursively process children
    for (const auto& child : node->children) {
        traverseNodeHierarchy(child.get(), globalTransformation, boneTransforms);
    }
}

cMat4 VKAnimation::getNodeTransformation(const cString& nodeName, double time) const
{
    if (animations_.empty())
        return cMat4(1.0f);

    const AnimationClip& currentAnim = animations_[currentAnimationIndex];

    // Find animation channel for this node
    for (const auto& channel : currentAnim.channels) {
        if (channel.nodeName == nodeName) {
            cVec3 position = channel.interpolatePosition(time);
            cQuat rotation = channel.interpolateRotation(time);
            cVec3 scale = channel.interpolateScale(time);

            cMat4 translation = glm::translate(cMat4(1.0f), position);
            cMat4 rotationMat = glm::mat4_cast(rotation);
            cMat4 scaleMat = glm::scale(cMat4(1.0f), scale);

            return translation * rotationMat * scaleMat;
        }
    }

    return cMat4(1.0f);
}

// Add this new method to build the scene graph
void VKAnimation::buildSceneGraph(const aiScene* scene)
{
    if (!scene || !scene->mRootNode)
        return;

    PRINT_TO_LOGGER("Building animation scene graph...");
    nodeMapping.clear();
    rootNode = buildSceneNode(scene->mRootNode, nullptr);
    PRINT_TO_LOGGER("Scene graph built with {} nodes", nodeMapping.size());
}

std::unique_ptr<VKAnimation::SceneNode> VKAnimation::buildSceneNode(const aiNode* aiNode, SceneNode* parent)
{
    auto node = std::make_unique<SceneNode>(aiNode->mName.C_Str());
    node->transformation = glm::transpose(glm::make_mat4(&aiNode->mTransformation.a1));
    node->parent = parent;

    // Add to quick lookup map
    nodeMapping[node->name] = node.get();

    // Process children
    for (uint32_t i = 0; i < aiNode->mNumChildren; ++i) {
        auto child = buildSceneNode(aiNode->mChildren[i], node.get());
        node->children.push_back(std::move(child));
    }

    return node;
}

// AnimationChannel interpolation methods
cVec3 AnimationChannel::interpolatePosition(double time) const
{
    return interpolateKeys(positionKeys, time);
}

cQuat AnimationChannel::interpolateRotation(double time) const
{
    if (rotationKeys.empty())
        return cQuat(1.0f, 0.0f, 0.0f, 0.0f);
    if (rotationKeys.size() == 1)
        return rotationKeys[0].value;

    // Find surrounding keyframes
    uint32_t index = 0;
    for (uint32_t i = 0; i < rotationKeys.size() - 1; ++i) {
        if (time < rotationKeys[i + 1].time) {
            index = i;
            break;
        }
    }

    if (index >= rotationKeys.size() - 1) {
        return rotationKeys.back().value;
    }

    const auto& key1 = rotationKeys[index];
    const auto& key2 = rotationKeys[index + 1];

    double deltaTime = key2.time - key1.time;
    float factor = static_cast<float>((time - key1.time) / deltaTime);

    return glm::slerp(key1.value, key2.value, factor);
}

cVec3 AnimationChannel::interpolateScale(double time) const
{
    return interpolateKeys(scaleKeys, time);
}

template <typename T>
T AnimationChannel::interpolateKeys(const std::vector<AnimationKey<T>>& keys, double time) const
{
    if (keys.empty())
        return T{};
    if (keys.size() == 1)
        return keys[0].value;

    // Find surrounding keyframes
    uint32_t index = 0;
    for (uint32_t i = 0; i < keys.size() - 1; ++i) {
        if (time < keys[i + 1].time) {
            index = i;
            break;
        }
    }

    if (index >= keys.size() - 1) {
        return keys.back().value;
    }

    const auto& key1 = keys[index];
    const auto& key2 = keys[index + 1];

    double deltaTime = key2.time - key1.time;
    float factor = static_cast<float>((time - key1.time) / deltaTime);

    return glm::mix(key1.value, key2.value, factor);
}

// Getters
float VKAnimation::getDuration() const
{
    if (animations_.empty())
        return 0.0f;
    const auto& currentAnim = animations_[currentAnimationIndex];
    return static_cast<float>(currentAnim.duration / currentAnim.ticksPerSecond);
}

const cString& VKAnimation::getCurrentAnimationName() const
{
    static const cString empty = "";
    if (animations_.empty())
        return empty;
    return animations_[currentAnimationIndex].name;
}

void VKAnimation::setAnimationIndex(cUint32_t index)
{
    if (index < animations_.size()) {
        currentAnimationIndex = index;
        currentTime = 0.0f;
    }
}

void VKAnimation::setPlaybackSpeed(float speed)
{
    playbackSpeed = speed;
}

void VKAnimation::setLooping(bool loop)
{
    isLooping = loop;
}

} // namespace vkengine
