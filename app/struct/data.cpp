#include "data.h"

const std::vector<Vertex> DepthTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}};

const std::vector<Vertex> SquareTestVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
};

const std::vector<Vertex> cube{
    // right face
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

    // left face
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

    // top face
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

    // bottom face
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 3.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 3.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 3.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

    // front face
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

    // back face
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 5.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 5.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}};

// https://github.com/SaschaWillems/Vulkan 에서 가져온 스카이박스 버텍스 데이터
// position , normal, texture coordinate
const std::vector<Vertex> skyboxVertices = {
    // back face
    {{-5.00f, 5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {1.00f, 0.00f, 0.0f}},
    {{5.00f, -5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {0.00f, 0.00f, 0.0f}},
    {{5.00f, 5.00f, -5.00f}, {0.00f, -1.19209e-07f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    // front face
    {{-5.00f, 5.00f, 5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {0.00f, 1.00f, 0.0f}},
    {{5.00f, 5.00f, 5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {1.00f, 1.00f, 0.0f}},
    {{5.00f, -5.00f, 5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f, -5.00f, 5.00f}, {0.00f, 1.19209e-07f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // top face
    {{-5.00f, 5.00f, -5.00f}, {0.00005f, -1.62921e-07f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{5.00f, 5.00f, -5.00f}, {-0.00005f, -1.62921e-07f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{5.00f, 5.00f, 5.00f}, {-0.00005f, 7.54979e-08f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f, 5.00f, 5.00f}, {0.00005f, 7.54979e-08f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // right face
    {{5.00f, 5.00f, -5.00f}, {-4.37114e-08f, -0.00005f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{5.00f, -5.00f, -5.00f}, {-4.37114e-08f, 0.00005f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{5.00f, -5.00f, 5.00f}, {-4.37114e-08f, 0.00005f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{5.00f, 5.00f, 5.00f}, {-4.37114e-08f, -0.00005f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // bottom face
    {{5.00f, -5.00f, -5.00f}, {-0.00005f, -7.54979e-08f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f, -5.00f}, {0.00005f, -7.54979e-08f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f, -5.00f, 5.00f}, {0.00005f, 1.62921e-07f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{5.00f, -5.00f, 5.00f}, {-0.00005f, 1.62921e-07f, 1.00f}, {0.00f, 0.00f, 0.0f}},
    // left face
    {{-5.00f, -5.00f, -5.00f}, {4.37114e-08f, 0.00005f, -1.00f}, {0.00f, 1.00f, 0.0f}},
    {{-5.00f, 5.00f, -5.00f}, {4.37114e-08f, -0.00005f, -1.00f}, {1.00f, 1.00f, 0.0f}},
    {{-5.00f, 5.00f, 5.00f}, {4.37114e-08f, -0.00005f, 1.00f}, {1.00f, 0.00f, 0.0f}},
    {{-5.00f, -5.00f, 5.00f}, {4.37114e-08f, 0.00005f, 1.00f}, {0.00f, 0.00f, 0.0f}},
};

const std::vector<cUint32_t> DepthTestIndices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4};

const std::vector<cUint32_t> SquareTestIndices_ = {
    0, 1, 2, 2, 3, 0};

const std::vector<cUint32_t> cubeindices_ = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20};

const std::vector<cUint32_t> skyboxIndices = {
    0, 1, 2, 2, 3, 0,       // left face
    4, 5, 6, 6, 7, 4,       // right face
    8, 9, 10, 10, 11, 8,    // top face
    12, 13, 14, 14, 15, 12, // bottom face
    16, 17, 18, 18, 19, 16, // back face
    20, 21, 22, 22, 23, 20  // front face
};

const std::vector<const cChar *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const cChar *> coreDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,        // 스왑체인 확장
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME // 동적 랜더링 확장자
};

// 추후 어떻게 사용할지 고민
// const std::vector<const cChar*> enabledDeviceExtensions = {};

const std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR};

const cString RESOURSE_PATH = "../../../../../../resource/";
const cString SHADER_PATH = "../../../../../../shader/";
const cString MODEL_PATH = "viking_room.obj";
const cString TEXTURE_PATH = "viking_room.png";
const cString TEST_TEXTURE_PATH = "image.png";
const cString TEST_TEXTURE_PATH_ARRAY0 = "Texture/512x512/Elements/Elements_15-512x512.png";
const cString TEST_TEXTURE_PATH_ARRAY1 = "Texture/512x512/Elements/Elements_16-512x512.png";
const cString TEST_TEXTURE_PATH_ARRAY2 = "Texture/512x512/Elements/Elements_17-512x512.png";
const cString TEST_TEXTURE_PATH_ARRAY3 = "Texture/512x512/Elements/Elements_18-512x512.png";
const cString TEST_TEXTURE_PATH_ARRAY4 = "Texture/512x512/Elements/Elements_19-512x512.png";
const cString TEST_TEXTURE_PATH_ARRAY5 = "Texture/512x512/Elements/Elements_20-512x512.png";
const cString CUBE_TEXTURE_PATH = "Texture/skybox";

size_t BindingHash::operator()(const std::vector<VkDescriptorSetLayoutBinding> &bindings) const
{
    size_t hash = 0;
    for (const auto &binding : bindings)
    {
        // Combine hash values for each field
        hash ^= std::hash<uint32_t>{}(binding.binding) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^=
            std::hash<uint32_t>{}(binding.descriptorType) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^=
            std::hash<uint32_t>{}(binding.descriptorCount) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<const void *>{}(binding.pImmutableSamplers) + 0x9e3779b9 + (hash << 6) +
                (hash >> 2);
    }
    return hash;
}

bool BindingEqual::operator()(
    const std::vector<VkDescriptorSetLayoutBinding> &lhs,
    const std::vector<VkDescriptorSetLayoutBinding> &rhs) const
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i)
    {
        const auto &l = lhs[i];
        const auto &r = rhs[i];

        // Note: stageFlags is not used to determine equality intentionally.

        if (l.binding != r.binding || l.descriptorType != r.descriptorType ||
            l.descriptorCount != r.descriptorCount ||
            /*  l.stageFlags != r.stageFlags || */
            l.pImmutableSamplers != r.pImmutableSamplers)
        {
            return false;
        }
    }
    return true;
}

#ifdef DEBUG_
const cBool enableValidationLayers = true;
#else
const cBool enableValidationLayers = false;
#endif