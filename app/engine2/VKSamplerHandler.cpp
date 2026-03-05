#include "VKSamplerHandler.h"
#include "log.h"

using namespace vkengine::Log;

namespace vkengine
{

    VKSamplerHandler::VKSamplerHandler(VKcontext &context) : ctx(context)
    {
        Log::PRINT_TO_LOGGER("VK Sampler created.\n");
        sampler = VK_NULL_HANDLE;
    }

    VKSamplerHandler::~VKSamplerHandler()
    {
        this->cleanup();
        Log::PRINT_TO_LOGGER("VK Sampler destroyed.\n");
    }
    
    VkSampler VKSamplerHandler::getSampler() const { return sampler; }

    void VKSamplerHandler::createAnisoRepeat()
    {
        cleanup();

        VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;

        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(this->ctx.getDevice()->physicalDevice, &deviceFeatures);

        if (deviceFeatures.samplerAnisotropy)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(this->ctx.getDevice()->physicalDevice, &properties);

            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        }
        else
        {
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = 1.0f;
            PRINT_TO_LOGGER("Warning: Anisotropic filtering is not supported on this device.\n");
        }

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        _VK_CHECK_RESULT_(vkCreateSampler(ctx.getDevice()->logicaldevice, &samplerInfo, nullptr, &sampler));
    }

    void VKSamplerHandler::createAnisoClamp()
    {
        cleanup();

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures deviceFeatures{};

        vkGetPhysicalDeviceProperties(this->ctx.getDevice()->physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(this->ctx.getDevice()->physicalDevice, &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;

        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerInfo.anisotropyEnable = VK_TRUE;

        if (deviceFeatures.samplerAnisotropy)
        {
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        }
        else
        {
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = 1.0f;
            PRINT_TO_LOGGER("Warning: Anisotropic filtering is not supported on this device.");
        }

        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        _VK_CHECK_RESULT_(vkCreateSampler(ctx.getDevice()->logicaldevice, &samplerInfo, nullptr, &sampler));
    }

    void VKSamplerHandler::createLinearRepeat()
    {
        cleanup();

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures deviceFeatures{};

        vkGetPhysicalDeviceProperties(this->ctx.getDevice()->physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(this->ctx.getDevice()->physicalDevice, &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;

        // Filtering
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        // Address modes - use REPEAT for normal texture wrapping
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // LOD settings
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // Allow all mip levels

        // No anisotropy for this sampler
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        // Comparison settings (typically for shadow mapping)
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // Border color (only used with CLAMP_TO_BORDER)
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        // Normalized coordinates
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        _VK_CHECK_RESULT_(vkCreateSampler(ctx.getDevice()->logicaldevice, &samplerInfo, nullptr, &sampler));
    }

    void VKSamplerHandler::createLinearClamp()
    {
        cleanup();

        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures deviceFeatures{};

        vkGetPhysicalDeviceProperties(this->ctx.getDevice()->physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(this->ctx.getDevice()->physicalDevice, &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;

        // Filtering
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        // Address modes - use CLAMP_TO_EDGE to prevent wrapping
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        // LOD settings
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // Allow all mip levels

        // No anisotropy for this sampler
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        // Comparison settings (typically for shadow mapping)
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // Border color (only used with CLAMP_TO_BORDER)
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        // Normalized coordinates
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        _VK_CHECK_RESULT_(vkCreateSampler(ctx.getDevice()->logicaldevice, &samplerInfo, nullptr, &sampler));
    }

    void VKSamplerHandler::createShadowMapSampler()
    {
        cleanup();
        
        VkPhysicalDeviceProperties properties{};
        VkPhysicalDeviceFeatures deviceFeatures{};

        vkGetPhysicalDeviceProperties(this->ctx.getDevice()->physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(this->ctx.getDevice()->physicalDevice, &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;

        // Filtering
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

        // LOD settings
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f; // No mipmapping for shadow maps

        // No anisotropy for this sampler
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        // Address modes - use CLAMP_TO_EDGE to prevent wrapping
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // Outside shadow map = fully lit
        
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // For shadow comparison

        _VK_CHECK_RESULT_(vkCreateSampler(ctx.getDevice()->logicaldevice, &samplerInfo, nullptr, &sampler));
    }

    void VKSamplerHandler::cleanup()
    {
        if (sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(ctx.getDevice()->logicaldevice, sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
    }
}
