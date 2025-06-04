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

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
    cVec3 pos;
    cVec3 color;
    cVec3 texCoord;

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
    // ��ġ�� ������ ��Ÿ���� �� ���� ��Ʈ����Ʈ�� �ֽ��ϴ�.
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    cBool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
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
    cVec2 position;
    cVec2 velocity;
    cVec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Particle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Particle, color);

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

struct UniformBufferObject {
    cMat4 model;
    cMat4 view;
    cMat4 proj;
};

struct TextureResource {

    cUint32_t texWidth = 0;                         // �ؽ�ó �ʺ�
    cUint32_t texHeight = 0;                        // �ؽ�ó ����
    cUint32_t texChannels = 0;                      // �ؽ�ó ä��
    cUChar* data = nullptr;         //< ���ҽ� ������ ������

    // ������
    TextureResource() {
        this->texWidth = 0;
        this->texHeight = 0;
        this->texChannels = 0;
        this->data = nullptr; // ���ҽ� ������ �ʱ�ȭ
    }

    // �Ҹ���
    ~TextureResource() {
        if (data) {
            free(data); // ���ҽ� ������ ����
        }
        texWidth = 0; // �ʺ� �ʱ�ȭ
        texHeight = 0; // ���� �ʱ�ȭ
        texChannels = 0; // ä�� �ʱ�ȭ
        data = nullptr; // ������ �ʱ�ȭ
    }


    // ���� ������
    TextureResource(const TextureResource& other) {
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
    TextureResource& operator=(const TextureResource& other) {
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
                return *this; // �޸� �Ҵ� ���� �� ���� ��ü ��ȯ
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

        this->texChannels = 4; // �⺻������ RGBA�� ����

        data = vkengine::load_png_rgba(path.c_str(), &this->texWidth, &this->texHeight, this->texChannels);
    }
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
