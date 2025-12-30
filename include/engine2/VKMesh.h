#ifndef VK_MESH_INCLUDE_H_
#define VK_MESH_INCLUDE_H_

#include "common.h"
#include "struct.h"

#include "VKContext.h"
#include "VKMaterial.h"
#include "VKViewFrustum.h"
#include "VKbuffer2.h"

// 홍정모 vulkan 그래픽스 Mesh 파일 프로젝트 사용

namespace vkengine {

class Mesh
{
  public:
    explicit Mesh(VKcontext& ctx) 
        : vertex(std::make_unique<VKBaseBuffer2>(ctx))
        , index(std::make_unique<VKBaseBuffer2>(ctx))
        , materialIndex(0)
        , minBounds(cVec3(FLT_MAX))
        , maxBounds(cVec3(-FLT_MAX))
        , isCulled(false)
        , noTextureCoords(false)
    {}

    Mesh() :
        vertex(nullptr)
        , index(nullptr)
        , materialIndex(0)
        , minBounds(cVec3(FLT_MAX))
        , maxBounds(cVec3(-FLT_MAX))
        , isCulled(false)
        , noTextureCoords(false)
    {}

    Mesh(Mesh&& other) noexcept
        : 
          vertex(std::move(other.vertex)), index(std::move(other.index)),
          name(std::move(other.name)), vertices(std::move(other.vertices)),
          indices(std::move(other.indices)), materialIndex(other.materialIndex),
          /*vertexBuffer(other.vertexBuffer), vertexMemory(other.vertexMemory),
          indexBuffer(other.indexBuffer), indexMemory(other.indexMemory),*/
          minBounds(other.minBounds), maxBounds(other.maxBounds), worldBounds(other.worldBounds),
          isCulled(other.isCulled), noTextureCoords(other.noTextureCoords)
    {
        // Reset moved-from object to safe state
        //other.vertexBuffer = VK_NULL_HANDLE;
        //other.vertexMemory = VK_NULL_HANDLE;
        //other.indexBuffer = VK_NULL_HANDLE;
        //other.indexMemory = VK_NULL_HANDLE;
        other.vertex = nullptr;
        other.index = nullptr;
        other.materialIndex = 0;
        other.minBounds = cVec3(FLT_MAX);
        other.maxBounds = cVec3(-FLT_MAX);
        other.isCulled = false;
        other.noTextureCoords = false;
    }

    Mesh& operator=(Mesh&& other) noexcept
    {
        if (this != &other) {
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

            // Transfer Vulkan resource ownership
            //vertexBuffer = other.vertexBuffer;
            //vertexMemory = other.vertexMemory;
            //indexBuffer = other.indexBuffer;
            //indexMemory = other.indexMemory;

            // Copy other members
            minBounds = other.minBounds;
            maxBounds = other.maxBounds;
            worldBounds = other.worldBounds;
            isCulled = other.isCulled;
            noTextureCoords = other.noTextureCoords;

            // Reset moved-from object to safe state
            //other.vertexBuffer = VK_NULL_HANDLE;
            //other.vertexMemory = VK_NULL_HANDLE;
            //other.indexBuffer = VK_NULL_HANDLE;
            //other.indexMemory = VK_NULL_HANDLE;
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
    void initializeBuffers(VKcontext& ctx) {
        if (!vertex) vertex = std::make_unique<VKBaseBuffer2>(ctx);
        if (!index) index = std::make_unique<VKBaseBuffer2>(ctx);
    }

    cString name = {};
    std::vector<Vertex2> vertices{};
    std::vector<cUint32_t> indices{};
    cUint32_t materialIndex = 0;

    // Vulkan buffers
    std::unique_ptr<VKBaseBuffer2> vertex;
    std::unique_ptr<VKBaseBuffer2> index;

    //VkBuffer vertexBuffer = VK_NULL_HANDLE;
    //VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    //VkBuffer indexBuffer = VK_NULL_HANDLE;
    //VkDeviceMemory indexMemory = VK_NULL_HANDLE;

    // Bounding box for culling
    cVec3 minBounds = cVec3(FLT_MAX);
    cVec3 maxBounds = cVec3(-FLT_MAX);

    void createBuffers(VKcontext& ctx);
    void cleanup(VkDevice device);
    void calculateBounds(); // Made public

    // Update Mesh::updateWorldBounds implementation
    void updateWorldBounds(const glm::mat4& modelMatrix);

    // World-space bounding box (updated when model matrix changes)
    AABB worldBounds{};

    // Check if mesh should be culled
    bool isCulled = false;
    bool noTextureCoords = false;

    // Binary file I/O methods
    bool readFromBinaryFileStream(std::ifstream& stream);
    bool writeToBinaryFileStream(std::ofstream& stream) const;

  private:
    // Helper methods for binary I/O
    template <typename T>
    bool writeValue(std::ofstream& stream, const T& value) const;

    template <typename T>
    bool readValue(std::ifstream& stream, T& value);

    bool writeString(std::ofstream& stream, const cString& str) const;
    bool readString(std::ifstream& stream, cString& str);

    template <typename T>
    bool writeVector(std::ofstream& stream, const std::vector<T>& vec) const;

    template <typename T>
    bool readVector(std::ifstream& stream, std::vector<T>& vec);
};

// Template implementations (must be in header for C++14)
template <typename T>
bool Mesh::writeValue(std::ofstream& stream, const T& value) const
{
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    return stream.good();
}

template <typename T>
bool Mesh::readValue(std::ifstream& stream, T& value)
{
    stream.read(reinterpret_cast<char*>(&value), sizeof(T));
    return stream.good();
}

template <typename T>
bool Mesh::writeVector(std::ofstream& stream, const std::vector<T>& vec) const
{
    uint32_t size = static_cast<uint32_t>(vec.size());
    if (!writeValue(stream, size))
        return false;

    if (size > 0) {
        stream.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
        return stream.good();
    }
    return true;
}

template <typename T>
bool Mesh::readVector(std::ifstream& stream, std::vector<T>& vec)
{
    uint32_t size;
    if (!readValue(stream, size))
        return false;

    vec.resize(size);
    if (size > 0) {
        stream.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
        return stream.good();
    }
    return true;
}

} // 

#endif // VK_MESH_INCLUDE_H_
