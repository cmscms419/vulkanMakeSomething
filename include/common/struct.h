#ifndef INCLUDE_STRUCT_H_
#define INCLUDE_STRUCT_H_

#include "common.h"
#include "macros.h"
#include "resourseload.h"



struct QueueFamilyIndices {
    cUint32_t graphicsAndComputeFamily = 0;  // �׷��Ƚ�/��ǻ�� ť �йи� �ε��� (�׷��Ƚ�/��ǻ�� ����� ó���ϴ� ť)
    cUint32_t presentFamily = 0;             // ������Ʈ ť �йи� �ε��� (������ �ý��۰� Vulkan�� �����ϴ� �������̽�)
    VkQueueFamilyProperties queueFamilyProperties = {};

    cBool graphicsAndComputeFamilyHasValue = false;
    cBool presentFamilyHasValue = false;

    void setgraphicsAndComputeFamily(cUint32_t index) {
        graphicsAndComputeFamily = index;
        graphicsAndComputeFamilyHasValue = true;
    }
    void setPresentFamily(cUint32_t index) {
        presentFamily = index;
        presentFamilyHasValue = true;
    }
    cUint32_t getGraphicsQueueFamilyIndex() const {
        cUint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsAndComputeFamily;
        }

        return target;
    }
    cUint32_t getPresentQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }
    cUint32_t getComputeQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
        {
            target = graphicsAndComputeFamily;
        }
        return target;
    }
    cBool isComplete() const {
        return this->graphicsAndComputeFamilyHasValue && this->presentFamilyHasValue;
    }
    void reset() {
        this->graphicsAndComputeFamily = 0;
        this->presentFamily = 0;
        this->graphicsAndComputeFamilyHasValue = false;
        this->presentFamilyHasValue = false;
        this->queueFamilyProperties = {};
    }
};

struct QueueFamilyIndices2 {
    cUint32_t grapicFamily = 0;  // �׷��Ƚ� ť �йи� �ε��� (�׷��Ƚ� ����� ó���ϴ� ť)
    cUint32_t computerFamily = 0; // ��ǻ�� ť �йи� �ε��� (��ǻ�� ����� ó���ϴ� ť)
    cUint32_t transferFamily = 0; // Ʈ������ ť �йи� �ε��� (Ʈ������ ����� ó���ϴ� ť)
    VkQueueFamilyProperties queueFamilyProperties = {};

    cBool grapicFamilyHasValue = false;
    cBool computerFamilyHasValue = false;
    cBool transferFamilyHasValue = false;

    void setGrapicFamily(cUint32_t index) {
        grapicFamily = index;
        grapicFamilyHasValue = true;
    }
    void setComputerFamily(cUint32_t index) {
        computerFamily = index;
        computerFamilyHasValue = true;
    }
    void setTransferFamily(cUint32_t index) {
        transferFamily = index;
        transferFamilyHasValue = true;
    }

    cUint32_t getGrapicQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = grapicFamily;
        }
        return target;
    }
    cUint32_t getComputerQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
        {
            target = computerFamily;
        }
        return target;
    }
    cUint32_t getTransferQueueFamilyIndex() const {
        cUint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
        {
            target = transferFamily;
        }
        return target;
    }

    cBool isComplete() const {
        return this->grapicFamilyHasValue && this->computerFamilyHasValue && this->transferFamilyHasValue;
    }
    void reset() {
        this->grapicFamily = 0;
        this->computerFamily = 0;
        this->transferFamily = 0;
        this->grapicFamilyHasValue = false;
        this->computerFamilyHasValue = false;
        this->transferFamilyHasValue = false;
    }
};


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    cVec3 pos;
    cVec3 normal;
    cVec3 texCoord;
    cVec4 inTangent;

    // ���ε� ������ ��ȯ�ϴ� �Լ�
    // �� ����ü�� ��� ������ ��� ���ε��Ǵ��� �����մϴ�.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // ��Ʈ����Ʈ ������ ��ȯ�ϴ� �Լ�
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, inTangent);

        return attributeDescriptions;
    }

    cBool operator==(const Vertex& other) const {
        return
            pos == other.pos &&
            normal == other.normal &&
            texCoord == other.texCoord &&
            inTangent == other.inTangent;
    }

};

struct subUniformBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* Mapped;

    void cleanup(VkDevice device) const {
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

struct Particle {
    cVec3 position;
    cVec3 velocity;
    cVec3 color;
    cVec4 empty;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Particle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Particle, velocity);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Particle, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Particle, empty);

        return attributeDescriptions;
    }
};

struct UniformBufferTime {
    cFloat deltaTime = 1.0f;
};

struct depthStencill {
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    VkImage depthImage{ VK_NULL_HANDLE };
    VkDeviceMemory depthImageMemory{ VK_NULL_HANDLE };
    VkImageView depthImageView{ VK_NULL_HANDLE };

    void cleanup(VkDevice device) const {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);
    }
};

struct FrameData {
    FrameData() {
        mainCommandBuffer = VK_NULL_HANDLE;
        VkimageavailableSemaphore = VK_NULL_HANDLE;
        VkrenderFinishedSemaphore = VK_NULL_HANDLE;
        VkinFlightFences = VK_NULL_HANDLE;
    }

    VkCommandBuffer mainCommandBuffer;
    VkSemaphore VkimageavailableSemaphore;
    VkSemaphore VkrenderFinishedSemaphore;
    VkFence VkinFlightFences;

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        info.pInheritanceInfo = nullptr;
        return info;
    }
};

struct ComputerFrameData {

    ComputerFrameData() {
        mainCommandBuffer = VK_NULL_HANDLE;
        computeFinishedSemaphores = VK_NULL_HANDLE;
        VkinFlightFences = VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        info.pInheritanceInfo = nullptr;
        return info;
    }

    VkCommandBuffer mainCommandBuffer;
    VkSemaphore computeFinishedSemaphores;
    VkFence VkinFlightFences;
};


struct UniformBufferObject {
    cMat4 model;
    cMat4 inverseTranspose;
    cMat4 view;
    cMat4 proj;
};


struct TextureResourceBase
{
    enum TYPE
    {
        NONE = 0,          // ���ҽ� ����
        PNG = 1,          // PNG ���ҽ�
        KTX = 2,          // KTX ���ҽ�
    };

    cChar* name = "";                              // ���ҽ� �̸�
    TYPE type = NONE;                               // ���ҽ� Ÿ�� (PNG, KTX ��)
    cUint32_t texWidth = 0;                         // �ؽ�ó �ʺ�
    cUint32_t texHeight = 0;                        // �ؽ�ó ����
    cUint32_t texChannels = 0;                      // �ؽ�ó ä��
    cUint32_t mipLevels = 0;
    cUint32_t layerCount = 0;

    virtual bool createResource(cString path = "", cChar* name = "") = 0; // ���ҽ� ���� �Լ�, ��θ� ���ڷ� ����

};

struct TextureResourcePNG : public TextureResourceBase {

    cUChar* data = nullptr;         //< ���ҽ� ������ ������

    // ������
    TextureResourcePNG() {
        this->name = ""; // �̸� �ʱ�ȭ
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->data = nullptr; // ���ҽ� ������ �ʱ�ȭ
        this->type = TYPE::PNG; // ���ҽ� Ÿ���� PNG�� ����
    }

    // �Ҹ���
    ~TextureResourcePNG() {

        if (data) {
            free(data); // ���ҽ� ������ ����
        }
        texWidth = 0; // �ʺ� �ʱ�ȭ
        texHeight = 0; // ���� �ʱ�ȭ
        texChannels = 0; // ä�� �ʱ�ȭ
        data = nullptr; // ������ �ʱ�ȭ
    }

    // ���� ������
    TextureResourcePNG(const TextureResourcePNG& other) {
        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;

        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);

            if (data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for texture data.");
                return; // �޸� �Ҵ� ���� �� �Լ� ����
            }

            memcpy(data, other.data, size);
        }
        else {
            data = nullptr;
        }
    }

    // ���� ������
    TextureResourcePNG& operator=(const TextureResourcePNG& other) {
        if (this == &other) return *this;
        if (data) free(data);

        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;
        name = other.name;

        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);

            if (data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for texture data.");
                return *this; // �޸� �Ҵ� ���� �� ���� ��ü ��ȯ
            }

            memcpy(data, other.data, size);
        }
        else {
            data = nullptr;
        }

        return *this;
    }

    virtual bool createResource(cString path = "", cChar* name = "") {

        this->name = name;

        if (data) {
            free(data);
        }

        this->texChannels = TextureType::Texture_rgb_alpha; // �⺻������ RGBA�� ����

        data = load_png_rgba(path.c_str(), &this->texWidth, &this->texHeight, this->texChannels);

        if (data == nullptr) {
            _PRINT_TO_CONSOLE_("PNG texture data is null.");
            return false;
        }

        return true;
    }
};

struct TextureResourceKTX : public TextureResourceBase {

    ktxTexture* texture = nullptr; // KTX �ؽ�ó ������

    TextureResourceKTX() {
        this->name = ""; // �̸� �ʱ�ȭ
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->mipLevels = 0;
        this->layerCount = 0;
        this->texture = nullptr; // KTX �ؽ�ó �ʱ�ȭ
        this->type = TYPE::KTX;
    }

    ~TextureResourceKTX() {
        if (texture) {
            ktxTexture_Destroy(texture); // KTX �ؽ�ó ����
        }
        texWidth = 0; // �ʺ� �ʱ�ȭ
        texHeight = 0; // ���� �ʱ�ȭ
        texChannels = 0; // ä�� �ʱ�ȭ
        mipLevels = 0;
        layerCount = 0;
        texture = nullptr; // ������ �ʱ�ȭ
    }

    virtual bool createResource(cString path = "", cChar* name = "") {
        if (texture) {
            ktxTexture_Destroy(texture); // KTX �ؽ�ó ����
        }

        this->texture = load_ktx_texture(path.c_str(), texture);

        if (texture == nullptr) {
            _PRINT_TO_CONSOLE_("KTX texture is null.");
            return false;
        }

        this->texWidth = texture->baseWidth;
        this->texHeight = texture->baseHeight;
        this->mipLevels = texture->numLevels;
        this->layerCount = texture->numLayers;
        this->texChannels = 4; // �⺻������ RGBA�� ����
        this->name = name;

        return true;
    }

};

// ������Ƽ��� ���� ��ο� �ݿ� ���� �����͸� �����մϴ�.
struct Primitive {
    cUint32_t firstIndex;
    cUint32_t indexCount;
    cInt32_t materialIndex;
};

// ����� (���� ����) �������� �����ϰ� ������ ���� �⺻ ��ҷ� ������ �� �ֽ��ϴ�.
struct Mesh {
    std::vector<::Primitive> primitives;
};

// A node represents an object in the glTF scene graph
struct Node {
    Node* parent;
    std::vector<Node*> children;
    Mesh mesh;
    cMat4 matrix;
    ~Node() {
        for (auto& child : children) {
            delete child;
        }
    }
};

struct cMaterial
{
    cFloat metallic = 0.0f; // Metallic factor
    cFloat roughness = 0.0f; // Roughness factor
    cFloat r = 0.0f; // Red color component
    cFloat g = 0.0f; // Green color component
    cFloat b = 0.0f; // Blue color component
    cFloat a = 1.0f; // Alpha component (opacity)
    VkBool32 isAlbedoMap = VK_FALSE; // Albedo map flag
    VkBool32 isNormalMap = VK_FALSE; // Normal map flag
    VkBool32 isMetallic = VK_FALSE; // Metallic flag
    VkBool32 isRoughness = VK_FALSE; // Roughness flag

    cUint32_t baseColorTextureIndex = 0;

#if 0
    cFloat emissive = 0.0f; // Emissive factor
    cFloat alphaCutoff = 0.5f; // Alpha cutoff value for transparency
    cBool isTransparent = false; // Transparency flag
    cBool isEmissive = false; // Emissive flag
    cBool isOcclusionMap = false; // Occlusion map flag
    cBool isSpecularMap = false; // Specular map flag
    cBool isEmissiveMap = false; // Emissive map flag
    cBool isAlphaCutoff = false; // Alpha cutoff flag
    cBool isAlphaBlend = false; // Alpha blend flag
    cBool isAlphaTest = false; // Alpha test flag
    cBool isDoubleSided = false; // Double-sided rendering flag
    cBool isUseTexture = false; // Texture usage flag
    cBool isUseNormalMap = false; // Normal map usage flag
    cBool isUseOcclusionMap = false; // Occlusion map usage flag
    cBool isUseAlbedoMap = false; // Albedo map usage flag
    cBool isUseSpecularMap = false; // Specular map usage flag
    cBool isUseEmissiveMap = false; // Emissive map usage flag
    cBool isUseAlphaCutoff = false; // Alpha cutoff usage flag
    cBool isUseAlphaBlend = false; // Alpha blend usage flag
    cBool isUseAlphaTest = false; // Alpha test usage flag
    cBool isUseDoubleSided = false; // Double-sided rendering usage flag
    cBool isUseMetallic = false; // Metallic usage flag
    cBool isUseRoughness = false; // Roughness usage flag
    cBool isUseEmissive = false; // Emissive usage flag
    cBool isUseColor = false; // Color usage flag
    cBool isUseSpecular = false; // Specular usage flag
    cBool isUseTextureResource = false; // Texture resourcePNG usage flag
#endif

    cMaterial(cFloat metallic = 0.0f, cFloat roughness = 0.0f, cVec4 color = cVec4(0.0f, 0.0f, 0.0f, 1.0f))
        : metallic(metallic), roughness(roughness), r(color.r), g(color.g), b(color.b), a(color.a) {

    }
};

struct UniformBufferSkymapParams
{
    cVec4 lightPos[4] = { cVec4(0.0f) }; // ���� ��ġ
    cFloat exposure = 0.0f;
    cFloat gamma = 0.0f;
};

extern const std::vector<Vertex> cube;
extern const std::vector<Vertex> skyboxVertices;
extern const std::vector<Vertex> DepthTestVertices;
extern const std::vector<Vertex> SquareTestVertices;
extern const std::vector<cUint32_t> DepthTestIndices;
extern const std::vector<cUint32_t> SquareTestIndices_;
extern const std::vector<cUint32_t> cubeindices_;
extern const std::vector<cUint32_t> skyboxIndices;

#endif // !INCLUDE_STRUCT_H_
