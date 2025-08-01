#ifndef INCLUDE_STRUCT_H_
#define INCLUDE_STRUCT_H_

#include "common.h"
#include "macros.h"
#include "resourseload.h"

struct QueueFamilyIndices {
    cUint32_t graphicsAndComputeFamily = 0;  // 그래픽스/컴퓨팅 큐 패밀리 인덱스 (그래픽스/컴퓨팅 명령을 처리하는 큐)
    cUint32_t presentFamily = 0;             // 프레젠트 큐 패밀리 인덱스 (윈도우 시스템과 Vulkan을 연결하는 인터페이스)
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

    // 바인딩 설명을 반환하는 함수
    // 이 구조체의 멤버 변수가 어떻게 바인딩되는지 설명합니다.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 어트리뷰트 설명을 반환하는 함수
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
    VkFormat depthFormat{};
    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};

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

struct TextureResourcePNG {

    cUint32_t texWidth = 0;                         // 텍스처 너비
    cUint32_t texHeight = 0;                        // 텍스처 높이
    cUint32_t texChannels = 0;                      // 텍스처 채널
    cUChar* data = nullptr;         //< 리소스 데이터 포인터

    // 생성자
    TextureResourcePNG() {
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->data = nullptr; // 리소스 데이터 초기화
    }

    // 소멸자
    ~TextureResourcePNG() {
        if (data) {
            free(data); // 리소스 데이터 해제
        }
        texWidth = 0; // 너비 초기화
        texHeight = 0; // 높이 초기화
        texChannels = 0; // 채널 초기화
        data = nullptr; // 포인터 초기화
    }


    // 복사 생성자
    TextureResourcePNG(const TextureResourcePNG& other) {
        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;

        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);

            if (data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for texture data.");
                return; // 메모리 할당 실패 시 함수 종료
            }

            memcpy(data, other.data, size);
        }
        else {
            data = nullptr;
        }
    }

    // 대입 연산자
    TextureResourcePNG& operator=(const TextureResourcePNG& other) {
        if (this == &other) return *this;
        if (!data) free(data);

        texWidth = other.texWidth;
        texHeight = other.texHeight;
        texChannels = other.texChannels;
        if (other.data) {
            size_t size = texWidth * texHeight * texChannels;
            data = (cUChar*)malloc(size);

            if (data == nullptr) {
                _PRINT_TO_CONSOLE_("Failed to allocate memory for texture data.");
                return *this; // 메모리 할당 실패 시 현재 객체 반환
            }

            memcpy(data, other.data, size);
        }
        else {
            data = nullptr;
        }

        return *this;
    }

    void createResource(cString path) {

        if (data) {
            free(data);
        }

        this->texChannels = TextureType::Texture_rgb_alpha; // 기본적으로 RGBA로 설정

        data = load_png_rgba(path.c_str(), &this->texWidth, &this->texHeight, this->texChannels);
    }
};

struct TextureResourceKTX {
    cUint32_t texWidth = 0;                         // 텍스처 너비
    cUint32_t texHeight = 0;                        // 텍스처 높이
    cUint32_t texChannels = 0;                      // 텍스처 채널
    cUint32_t mipLevels = 0;
    cUint32_t layerCount = 0;
    ktxTexture* texture = nullptr; // KTX 텍스처 포인터

    TextureResourceKTX() {
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->mipLevels = 0;
        this->layerCount = 0;
        this->texture = nullptr; // KTX 텍스처 초기화
    }

    ~TextureResourceKTX() {
        if (texture) {
            ktxTexture_Destroy(texture); // KTX 텍스처 해제
        }
        texWidth = 0; // 너비 초기화
        texHeight = 0; // 높이 초기화
        texChannels = 0; // 채널 초기화
        mipLevels = 0;
        layerCount = 0;
        texture = nullptr; // 포인터 초기화
    }

    bool createResource(cString path) {

        if (texture) {
            ktxTexture_Destroy(texture); // KTX 텍스처 해제
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
        this->texChannels = 4; // 기본적으로 RGBA로 설정

        return true;
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

    //cMaterial() {};
    cMaterial(cFloat metallic = 0.0f, cFloat roughness = 0.0f, cVec4 color = cVec4(0.0f, 0.0f, 0.0f, 1.0f))
        : metallic(metallic), roughness(roughness), r(color.r), g(color.g), b(color.b), a(color.a){

    }
};

struct UniformBufferSkymapParams
{
    cVec4 lightPos[4] = { cVec4(0.0f) }; // 조명 위치
    cFloat exposure = 0.0f;
    cFloat gamma = 0.0f;
};

extern const std::vector<Vertex> cube;
extern const std::vector<Vertex> skyboxVertices;
extern const std::vector<Vertex> DepthTestVertices;
extern const std::vector<Vertex> SquareTestVertices;
extern const std::vector<cUint16_t> DepthTestIndices;
extern const std::vector<cUint16_t> SquareTestIndices_;
extern const std::vector<cUint16_t> cubeindices_;
extern const std::vector<cUint16_t> skyboxIndices;

#endif // !INCLUDE_STRUCT_H_
