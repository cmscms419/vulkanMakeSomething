#ifndef INCLUDE_STRUCT_H_
#define INCLUDE_STRUCT_H_

#include "./_common.h"
#include "./math.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


struct QueueFamilyIndices {
    uint32_t graphicsAndComputeFamily = 0;  // 그래픽스/컴퓨팅 큐 패밀리 인덱스 (그래픽스/컴퓨팅 명령을 처리하는 큐)
    uint32_t presentFamily = 0;             // 프레젠트 큐 패밀리 인덱스 (윈도우 시스템과 Vulkan을 연결하는 인터페이스)
    VkQueueFamilyProperties queueFamilyProperties = {};

    bool graphicsAndComputeFamilyHasValue = false;
    bool presentFamilyHasValue = false;

    void setgraphicsAndComputeFamily(uint32_t index) {
        graphicsAndComputeFamily = index;
        graphicsAndComputeFamilyHasValue = true;
    }
    void setPresentFamily(uint32_t index) {
        presentFamily = index;
        presentFamilyHasValue = true;
    }
    const uint32_t getGraphicsQueueFamilyIndex() {
        uint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsAndComputeFamily;
        }

        return target;
    }
    const uint32_t getPresentQueueFamilyIndex() {
        uint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }
    const uint32_t getComputeQueueFamilyIndex() {
        uint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
        {
            target = graphicsAndComputeFamily;
        }
        return target;
    }
    const bool isComplete() {
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
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

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
    // 위치와 색상을 나타내는 두 개의 어트리뷰트가 있습니다.
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
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

};

struct VertexPosColor {
    glm::vec3 pos;
    glm::vec3 color;

    // 바인딩 설명을 반환하는 함수
    // 이 구조체의 멤버 변수가 어떻게 바인딩되는지 설명합니다.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexPosColor);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 어트리뷰트 설명을 반환하는 함수
    // 위치와 색상을 나타내는 두 개의 어트리뷰트가 있습니다.
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexPosColor, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexPosColor, color);

        return attributeDescriptions;
    }

    bool operator==(const VertexPosColor& other) const {
        return pos == other.pos && color == other.color;
    }

};

struct VertexBuffer {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexmemory;

    void cleanup(VkDevice device) {
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexMemory, nullptr);
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexmemory, nullptr);
    }
};

struct UniformBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* Mapped;

    void cleanup(VkDevice device) {
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }
};

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;

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
    float deltaTime = 1.0f;
};

struct depthStencill {
    VkFormat depthFormat{};
    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};

    void cleanup(VkDevice device) {
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

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

const std::vector<Vertex> testVectex = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<VertexPosColor> testVectex_ = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> testindices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
};

const std::vector<uint16_t> testindices_ = {
        0, 1, 2, 2, 3, 0
};

const std::vector<VertexPosColor> cube{

    // left face (white)
    {{-0.5f, -0.5f, -0.5f}, {.9f, .9f, .9f}},
    {{-0.5f, -0.5f, 0.5f}, {.9f, .9f, .9f}},
    {{-0.5f, 0.5f, 0.5f}, {.9f, .9f, .9f}},
    {{-0.5f, 0.5f, -0.5f}, {.9f, .9f, .9f}},

    // right face (yellow)
    {{0.5f, -0.5f, -0.5f}, {.8f, .8f, .1f}},
    {{0.5f, 0.5f, -0.5f}, {.8f, .8f, .1f}},
    {{0.5f, 0.5f, 0.5f}, {.8f, .8f, .1f}},
    {{0.5f, -0.5f, 0.5f}, {.8f, .8f, .1f}},

    // top face (orange, remember y axis points down)
    {{-0.5f, -0.5f, -0.5f}, {.9f, .6f, .1f}},
    {{0.5f, -0.5f, -0.5f}, {.9f, .6f, .1f}},
    {{0.5f, -0.5f, 0.5f}, {.9f, .6f, .1f}},
    {{-0.5f, -0.5f, 0.5f}, {.9f, .6f, .1f}},

    // bottom face (red)
    {{-0.5f, 0.5f, -0.5f}, {.8f, .1f, .1f}},
    {{-0.5f, 0.5f, 0.5f}, {.8f, .1f, .1f}},
    {{0.5f, 0.5f, 0.5f}, {.8f, .1f, .1f}},
    {{0.5f, 0.5f, -0.5f}, {.8f, .1f, .1f}},

    // nose face (blue)
    {{-0.5f, -0.5f, 0.5f}, {.1f, .1f, .8f}},
    {{0.5f, -0.5f, 0.5f}, {.1f, .1f, .8f}},
    {{0.5f, 0.5f, 0.5f}, {.1f, .1f, .8f}},
    {{-0.5f, 0.5f, 0.5f}, {.1f, .1f, .8f}},

    // tail face (green)
    {{-0.5f, -0.5f, -0.5f}, {.1f, .8f, .1f}},
    {{-0.5f, 0.5f, -0.5f}, {.1f, .8f, .1f}},
    {{0.5f, 0.5f, -0.5f}, {.1f, .8f, .1f}},
    {{0.5f, -0.5f, -0.5f}, {.1f, .8f, .1f}},
};

const std::vector<uint16_t> cubeindices_ = {
       0, 1, 2, 2, 3, 0,
       4, 5, 6, 6, 7, 4,
	   8, 9, 10, 10, 11, 8,
	   12, 13, 14, 14, 15, 12,
	   16, 17, 18, 18, 19, 16,
	   20, 21, 22, 22, 23, 20
};


namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };

    template<> struct hash<VertexPosColor> {
        size_t operator()(VertexPosColor const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                    (hash<glm::vec3>()(vertex.color) << 1)));
        }
    };
}



#endif // !INCLUDE_STRUCT_H_
