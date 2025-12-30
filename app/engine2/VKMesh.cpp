#include "VKMesh.h"
#include "helper.h"
#include "log.h"

#include <fstream>

using namespace vkengine::Log;

namespace vkengine {

void Mesh::createBuffers(VKcontext& ctx)
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

    VkDeviceMemory stagingBufferMemory;
    VkBuffer stagingBuffer;

    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = vertexBufferSize + indexBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    _VK_CHECK_RESULT_(vkCreateBuffer(ctx.getDevice()->logicaldevice, &bufferInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx.getDevice()->logicaldevice, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    _VK_CHECK_RESULT_(vkAllocateMemory(ctx.getDevice()->logicaldevice, &allocInfo, nullptr, &stagingBufferMemory));
    _VK_CHECK_RESULT_(vkBindBufferMemory(ctx.getDevice()->logicaldevice, stagingBuffer, stagingBufferMemory, 0));

    void* data;
    _VK_CHECK_RESULT_(vkMapMemory(ctx.getDevice()->logicaldevice, stagingBufferMemory, 0, vertexBufferSize + indexBufferSize, 0, &data));
    memcpy(data, vertices.data(), static_cast<size_t>(vertexBufferSize));
    memcpy(static_cast<char*>(data) + vertexBufferSize, indices.data(),
           static_cast<size_t>(indexBufferSize));
    vkUnmapMemory(ctx.getDevice()->logicaldevice, stagingBufferMemory);

    this->vertex->createModelVertexBuffer(vertexBufferSize, nullptr);
    this->index->createModeIndexBuffer(indexBufferSize, nullptr);

    /*bufferInfo.size = vertexBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    check(vkCreateBuffer(ctx.device(), &bufferInfo, nullptr, &vertexBuffer));

    vkGetBufferMemoryRequirements(ctx.device(), vertexBuffer, &memRequirements);
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    check(vkAllocateMemory(ctx.device(), &allocInfo, nullptr, &vertexMemory));
    check(vkBindBufferMemory(ctx.device(), vertexBuffer, vertexMemory, 0));

    bufferInfo.size = indexBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    check(vkCreateBuffer(ctx.device(), &bufferInfo, nullptr, &indexBuffer));

    vkGetBufferMemoryRequirements(ctx.device(), indexBuffer, &memRequirements);
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        ctx.getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    check(vkAllocateMemory(ctx.device(), &allocInfo, nullptr, &indexMemory));
    check(vkBindBufferMemory(ctx.device(), indexBuffer, indexMemory, 0));
    */

    VKCommandBufferHander cmd = ctx.createGrapicsCommandBufferHander(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copyRegion{};
    copyRegion.size = vertexBufferSize;
    vkCmdCopyBuffer(cmd.getCommandBuffer(), stagingBuffer, this->vertex->Buffer(), 1, &copyRegion);

    copyRegion.srcOffset = vertexBufferSize;
    copyRegion.dstOffset = 0;
    copyRegion.size = indexBufferSize;
    vkCmdCopyBuffer(cmd.getCommandBuffer(), stagingBuffer, this->index->Buffer(), 1, &copyRegion);

    cmd.submitAndWait();

    vkDestroyBuffer(ctx.getDevice()->logicaldevice, stagingBuffer, nullptr);
    vkFreeMemory(ctx.getDevice()->logicaldevice, stagingBufferMemory, nullptr);

    calculateBounds();
}

void Mesh::calculateBounds()
{
    minBounds = cVec3(FLT_MAX);
    maxBounds = cVec3(-FLT_MAX);

    for (const auto& vertex : vertices) {
        minBounds = glm::min(minBounds, vertex.pos);
        maxBounds = glm::max(maxBounds, vertex.pos);
    }
}

// Update Mesh::updateWorldBounds implementation
void Mesh::updateWorldBounds(const glm::mat4& modelMatrix)
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
bool Mesh::writeToBinaryFileStream(std::ofstream& stream) const
{
    // File format version for future compatibility
    const uint32_t fileVersion = 1;
    if (!writeValue(stream, fileVersion))
        return false;

    // Write mesh name
    if (!writeString(stream, name))
        return false;

    // Write material index
    if (!writeValue(stream, materialIndex))
        return false;

    // Write vertex data
    if (!writeVector(stream, vertices))
        return false;

    // Write index data
    if (!writeVector(stream, indices))
        return false;

    // Write bounding box
    if (!writeValue(stream, minBounds))
        return false;
    if (!writeValue(stream, maxBounds))
        return false;

    // Write flags
    if (!writeValue(stream, isCulled))
        return false;
    if (!writeValue(stream, noTextureCoords))
        return false;

    return stream.good();
}

bool Mesh::readFromBinaryFileStream(std::ifstream& stream)
{
    // Read and verify file format version
    uint32_t fileVersion;
    if (!readValue(stream, fileVersion))
        return false;

    if (fileVersion != 1) {
        std::cerr << "Unsupported mesh file version: " << fileVersion << std::endl;
        return false;
    }

    // Read mesh name
    if (!readString(stream, name))
        return false;

    // Read material index
    if (!readValue(stream, materialIndex))
        return false;

    // Read vertex data
    if (!readVector(stream, vertices))
        return false;

    // Read index data
    if (!readVector(stream, indices))
        return false;

    // Read bounding box
    if (!readValue(stream, minBounds))
        return false;
    if (!readValue(stream, maxBounds))
        return false;

    // Read flags
    if (!readValue(stream, isCulled))
        return false;
    if (!readValue(stream, noTextureCoords))
        return false;

    // Reset Vulkan handles (they need to be recreated)
    this->vertex.reset();
    this->index.reset();

    // Initialize world bounds from local bounds
    worldBounds = AABB(minBounds, maxBounds);

    return stream.good();
}

// Helper method implementations
bool Mesh::writeString(std::ofstream& stream, const cString& str) const
{
    uint32_t length = static_cast<uint32_t>(str.length());
    if (!writeValue(stream, length))
        return false;

    if (length > 0) {
        stream.write(str.c_str(), length);
        return stream.good();
    }
    return true;
}

bool Mesh::readString(std::ifstream& stream, cString& str)
{
    uint32_t length;
    if (!readValue(stream, length))
        return false;

    if (length > 0) {
        str.resize(length);
        stream.read(&str[0], length);
        return stream.good();
    } else {
        str.clear();
        return true;
    }
}

} // namespace vkengine
