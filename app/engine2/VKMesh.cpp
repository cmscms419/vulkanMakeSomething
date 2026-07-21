#include "VKMesh.h"

#include "helper.h"
#include "log.h"

#include <fstream>

using namespace vkengine::Log;

namespace vkengine
{
    Mesh::Mesh() :
        vertex(nullptr)
        , index(nullptr)
        , materialIndex(0)
        , minBounds(cVec3(FLT_MAX))
        , maxBounds(cVec3(-FLT_MAX))
        , isCulled(false)
        , noTextureCoords(false)
    {
        name = "UnnamedMesh";
        worldBounds = AABB();
    }

    Mesh::Mesh (VKcontext& ctx) : 
        vertex(std::make_unique<VKBaseBuffer2>(ctx))
        , index(std::make_unique<VKBaseBuffer2>(ctx))
        , materialIndex(0)
        , minBounds(cVec3(FLT_MAX))
        , maxBounds(cVec3(-FLT_MAX))
        , isCulled(false)
        , noTextureCoords(false)
    {
        name = "UnnamedMesh";
        worldBounds = AABB();
    }

    void Mesh::createBuffers(VKcontext &ctx)
    {
        if (this->vertex == nullptr || this->index == nullptr) {
            this->initializeBuffers(ctx);
        }

        VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
        VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

        this->vertex->createModelVertexBuffer(vertexBufferSize, nullptr);
        this->index->createModelIndexBuffer(indexBufferSize, nullptr);

        this->vertex->copyData(vertices.data(), vertexBufferSize);
        this->index->copyData(indices.data(), indexBufferSize);

        calculateBounds();
    }

    void Mesh::calculateBounds()
    {
        minBounds = cVec3(FLT_MAX);
        maxBounds = cVec3(-FLT_MAX);

        for (const auto &vertex : vertices)
        {
            minBounds = glm::min(minBounds, vertex.pos);
            maxBounds = glm::max(maxBounds, vertex.pos);
        }
    }

    // Update Mesh::updateWorldBounds implementation
    void Mesh::updateWorldBounds(const glm::mat4 &modelMatrix)
    {
        AABB localBounds(minBounds, maxBounds);
        worldBounds = localBounds.transform(modelMatrix);
    }

    void Mesh::cleanup(VkDevice device)
    {
        this->vertex.reset();
        this->index.reset();
    }

    // Binary file I/O implementation
    bool Mesh::writeToBinaryFileStream(std::ofstream &stream) const
    {
        // File format version for future compatibility
        const uint32_t fileVersion = 1;
        if (!helper::file::writeValue(stream, fileVersion))
            return false;

        // Write mesh name
        if (!helper::file::writeString(stream, name))
            return false;

        // Write material index
        if (!helper::file::writeValue(stream, materialIndex))
            return false;

        // Write vertex data
        if (!helper::file::writeVector(stream, vertices))
            return false;

        // Write index data
        if (!helper::file::writeVector(stream, indices))
            return false;

        // Write bounding box
        if (!helper::file::writeValue(stream, minBounds))
            return false;
        if (!helper::file::writeValue(stream, maxBounds))
            return false;

        // Write flags
        if (!helper::file::writeValue(stream, isCulled))
            return false;
        if (!helper::file::writeValue(stream, noTextureCoords))
            return false;

        return stream.good();
    }

    bool Mesh::readFromBinaryFileStream(std::ifstream &stream)
    {
        // Read and verify file format version
        uint32_t fileVersion;
        if (!helper::file::readValue(stream, fileVersion))
            return false;

        if (fileVersion != 1)
        {
            std::cerr << "Unsupported mesh file version: " << fileVersion << std::endl;
            return false;
        }

        // Read mesh name
        if (!helper::file::readString(stream, name))
            return false;

        // Read material index
        if (!helper::file::readValue(stream, materialIndex))
            return false;

        // Read vertex data
        if (!helper::file::readVector(stream, vertices))
            return false;

        // Read index data
        if (!helper::file::readVector(stream, indices))
            return false;

        // Read bounding box
        if (!helper::file::readValue(stream, minBounds))
            return false;
        if (!helper::file::readValue(stream, maxBounds))
            return false;

        // Read flags
        if (!helper::file::readValue(stream, isCulled))
            return false;
        if (!helper::file::readValue(stream, noTextureCoords))
            return false;

        // Reset Vulkan handles (they need to be recreated)
        this->vertex.reset();
        this->index.reset();

        // Initialize world bounds from local bounds
        worldBounds = AABB(minBounds, maxBounds);

        return stream.good();
    }

} // namespace vkengine
