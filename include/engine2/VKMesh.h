#ifndef VK_MESH_INCLUDE_H_
#define VK_MESH_INCLUDE_H_

#include "common.h"
#include "type.h"

#include "VKContext.h"
#include "VKMaterial.h"
#include "VKViewFrustum.h"
#include "VKbuffer2.h"

#include <memory>

// 홍정모 vulkan 그래픽스 Mesh 파일 프로젝트 사용

namespace vkengine
{
    class Mesh
    {
    public:
        explicit Mesh(VKcontext &ctx);
        Mesh();
        Mesh(Mesh &&other) noexcept
            : vertex(std::move(other.vertex)), index(std::move(other.index)),
              name(std::move(other.name)), vertices(std::move(other.vertices)),
              indices(std::move(other.indices)), materialIndex(other.materialIndex),
              minBounds(other.minBounds), maxBounds(other.maxBounds), worldBounds(other.worldBounds),
              isCulled(other.isCulled), noTextureCoords(other.noTextureCoords)
        {
            // Reset moved-from object to safe state
            other.vertex = nullptr;
            other.index = nullptr;
            other.materialIndex = 0;
            other.minBounds = cVec3(FLT_MAX);
            other.maxBounds = cVec3(-FLT_MAX);
            other.isCulled = false;
            other.noTextureCoords = false;
        }

        Mesh &operator=(Mesh &&other) noexcept
        {
            if (this != &other)
            {
                // Note: We cannot safely cleanup existing Vulkan resources here
                // because we don't have access to VkDevice
                // The user must call cleanup() before move assignment if needed

                vertex.reset();
                index.reset();

                // Move all data members
                vertex = std::move(other.vertex);
                index = std::move(other.index);
                name = std::move(other.name);
                vertices = std::move(other.vertices);
                indices = std::move(other.indices);
                materialIndex = other.materialIndex;

                // Copy other members
                minBounds = other.minBounds;
                maxBounds = other.maxBounds;
                worldBounds = other.worldBounds;
                isCulled = other.isCulled;
                noTextureCoords = other.noTextureCoords;

                // Reset moved-from object to safe state
                other.vertex = nullptr;
                other.index = nullptr;
                other.materialIndex = 0;
                other.minBounds = cVec3(FLT_MAX);
                other.maxBounds = cVec3(-FLT_MAX);
                other.isCulled = false;
                other.noTextureCoords = false;
            }
            return *this;
        }

        // 버퍼 초기화
        void initializeBuffers(VKcontext &ctx)
        {
            if (!vertex)
                vertex = std::make_unique<VKBaseBuffer2>(ctx);
            if (!index)
                index = std::make_unique<VKBaseBuffer2>(ctx);
        }

        cString name = {};
        std::vector<Vertex2> vertices;
        std::vector<cUint32_t> indices;
        cUint32_t materialIndex;

        // Vulkan buffers
        std::unique_ptr<VKBaseBuffer2> vertex;
        std::unique_ptr<VKBaseBuffer2> index;

        // Bounding box for culling
        cVec3 minBounds;
        cVec3 maxBounds;

        void createBuffers(VKcontext &ctx);
        void cleanup(VkDevice device);
        void calculateBounds(); // Made public

        // Update Mesh::updateWorldBounds implementation
        void updateWorldBounds(const glm::mat4 &modelMatrix);

        // World-space bounding box (updated when model matrix changes)
        AABB worldBounds;

        // Check if mesh should be culled
        bool isCulled = false;
        bool noTextureCoords = false;

        // Binary file I/O methods
        bool readFromBinaryFileStream(std::ifstream &stream);
        bool writeToBinaryFileStream(std::ofstream &stream) const;
    };

} // namespace vkengine

#endif // VK_MESH_INCLUDE_H_
