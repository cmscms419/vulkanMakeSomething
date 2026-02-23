#include "helperDevice.h"

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {
        namespace device
        {
            cBool isDeviceSuitableWithSurface(VkPhysicalDevice device, VkSurfaceKHR VKsurface, QueueFamilyIndices &indices)
            {
                QueueFamilyIndices indices_ = findQueueFamiliesWithSurface(device, VKsurface);
                indices = indices_;

                cBool extensionsSupported = checkDeviceExtensionSupport(device);
                cBool swapChainAdequate = false;

                if (extensionsSupported)
                {
                    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, VKsurface);
                    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
                }

                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

                return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
            }

            cBool isDeviceSuitableWithoutSurface(VkPhysicalDevice device, QueueFamilyIndices2 &indices)
            {
                // 1. 큐 패밀리 검사 (Surface 없이 그래픽스 큐만 확인)
                QueueFamilyIndices2 indices_ = findQueueFamiliesWithoutSurface(device);
                indices = indices_;

                // 2. 디바이스 확장 지원 검사
                cBool extensionsSupported = checkDeviceExtensionSupport(device);

                // 3. 물리 디바이스 특성 검사
                VkPhysicalDeviceFeatures supportedFeatures;
                vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

                // 4. 디바이스 속성 검사 (옵션)
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);

                // 스왑체인 검사는 제외하고 기본적인 적합성만 확인
                return indices_.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
            }

            cBool checkDeviceExtensionSupport(VkPhysicalDevice device)
            {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

                std::set<cString> requiredExtensions(coreDeviceExtensions.begin(), coreDeviceExtensions.end());

                for (const auto &extension : availableExtensions)
                {

                    PRINT_TO_LOGGER("Available Extension: %s\n", extension.extensionName);
                    requiredExtensions.erase(extension.extensionName);
                }

                return requiredExtensions.empty();
            }

            void getDeviceExtensionSupport(VkPhysicalDevice device, std::set<cString> *temp)
            {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

                for (const auto &extension : availableExtensions)
                {
                    temp->insert(extension.extensionName);
                }
            }
            
            int rateDeviceSuitability(VkPhysicalDevice device)
            {
                VkPhysicalDeviceProperties deviceProperties;
                VkPhysicalDeviceFeatures deviceFeatures;

                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

                int score = 0;

                // Discrete GPUs have a significant performance advantage
                if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    score += 1000;
                }

                // Maximum possible size of textures affects graphics quality
                score += deviceProperties.limits.maxImageDimension2D;

                // Application can't function without geometry shaders
                if (!deviceFeatures.geometryShader)
                {
                    return 0;
                }

                PRINT_TO_LOGGER("Device %s score: %d\n", deviceProperties.deviceName, score);
                PRINT_TO_LOGGER("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
                PRINT_TO_LOGGER("Device Name: %s\n", deviceProperties.deviceName);
                PRINT_TO_LOGGER("\n");

                return score;
            }

            const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR VKsurface)
            {
                SwapChainSupportDetails details;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VKsurface, &details.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, nullptr);

                if (formatCount != 0)
                {
                    details.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VKsurface, &formatCount, details.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, nullptr);

                if (presentModeCount != 0)
                {
                    details.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VKsurface, &presentModeCount, details.presentModes.data());
                }

                return details;
            }

            VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
            {
                return findSupportedFormat(
                    physicalDevice,
                    {VK_FORMAT_D32_SFLOAT,
                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                     VK_FORMAT_D24_UNORM_S8_UINT},                 // 후보 형식
                    VK_IMAGE_TILING_OPTIMAL,                       // 타일링
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT // 특징
                );
            }

            VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,const std::vector<VkFormat> &candidates,VkImageTiling tiling,VkFormatFeatureFlags features)
            {
                VkFormat value{VK_FORMAT_UNDEFINED};

                for (VkFormat format : candidates)
                {
                    VkFormatProperties props;
                    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                    {
                        value = format;
                    }
                    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                    {
                        value = format;
                    }
                    else
                    {
                        PRINT_TO_LOGGER("Failed to find supported format!\n");
                    }
                }

                return value;
            }

            uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
            {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                    {
                        return i;
                    }
                }

                return 0;
            }

            VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
            {
                VkSampleCountFlagBits value = VK_SAMPLE_COUNT_1_BIT;

                VkPhysicalDeviceProperties physicalDeviceProperties;
                vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

                VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

                if (counts & VK_SAMPLE_COUNT_64_BIT)
                {
                    value = VK_SAMPLE_COUNT_64_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_32_BIT)
                {
                    value = VK_SAMPLE_COUNT_32_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_16_BIT)
                {
                    value = VK_SAMPLE_COUNT_16_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_8_BIT)
                {
                    value = VK_SAMPLE_COUNT_8_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_4_BIT)
                {
                    value = VK_SAMPLE_COUNT_4_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_2_BIT)
                {
                    value = VK_SAMPLE_COUNT_2_BIT;
                }

                return value;
            }

            cString getPhysicalDeviceTypeString(VkPhysicalDeviceType type)
            {
                switch (type)
                {
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    return "Other";
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    return "Integrated GPU";
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    return "Discrete GPU";
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    return "Virtual GPU";
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    return "CPU";
                default:
                    return "Unknown";
                }
            }

            const QueueFamilyIndices findQueueFamiliesWithSurface(VkPhysicalDevice &device, VkSurfaceKHR &VKsurface)
            {
                QueueFamilyIndices indices; // 큐 패밀리의 개수를 저장할 변수를 초기화
                QueueFamilyIndices target;  // 큐 패밀리의 개수를 저장할 변수를 초기화

                // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (첫 번째 호출은 개수만 가져옴)
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

                // 주어진 물리 장치에서 큐 패밀리 속성을 가져옴 (두 번째 호출은 실제 속성을 가져옴)
                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef DEBUG_
                getPyhsicalDeviceProperties(device);
#endif // DEBUG_

                int i = 0;
                cBool selected = false;
                for (const auto &queueFamily : queueFamilies)
                {
                    // 현재 큐 패밀리가 그래픽스 큐를 지원하는지 확인
                    PRINT_TO_LOGGER("QueueFamily %d\n", i);
                    PRINT_TO_LOGGER("QueueFamily queueCount: %d\n", queueFamily.queueCount);

                    cString queueFlagsStr;
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        queueFlagsStr += "GRAPHICS ";
                    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                        queueFlagsStr += "COMPUTE ";
                    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                        queueFlagsStr += "TRANSFER ";
                    if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                        queueFlagsStr += "SPARSE_BINDING ";
                    if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT)
                        queueFlagsStr += "PROTECTED ";
                    if (queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                        queueFlagsStr += "VIDEO_DECODE ";
                    if (queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                        queueFlagsStr += "VIDEO_ENCODE ";
                    if (queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
                        queueFlagsStr += "OPTICAL_FLOW ";

                    PRINT_TO_LOGGER("QueueFamily queueFlags: %s\n", queueFlagsStr.c_str());

                    PRINT_TO_LOGGER("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);

                    if ((queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT))
                    {
                        PRINT_TO_LOGGER("VK_QUEUE_GRAPHICS_BIT is supported\n");
                        PRINT_TO_LOGGER("VK_QUEUE_COMPUTE_BIT is supported\n");
                        if (!selected)
                        {
                            indices.setgraphicsAndComputeFamily(i);
                        }
                    }

                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKsurface, &presentSupport);

                    if (presentSupport)
                    {
                        PRINT_TO_LOGGER("VK_QUEUE_PRESENT_BIT is supported\n");
                        if (!selected)
                        {
                            indices.setPresentFamily(i);
                        }
                    }

                    if (indices.isComplete())
                    {

                        if (!selected)
                        {
                            indices.queueFamilyProperties = queueFamily;
                            target = indices;
                            PRINT_TO_LOGGER("------------------ select Queuefamily index: %d ------------------\n", i);
                            selected = true;
                        }
                    }

                    indices.reset();
                    i++;
                }

                return target;
            }

            const QueueFamilyIndices2 findQueueFamiliesWithoutSurface(VkPhysicalDevice &device)
            {
                QueueFamilyIndices2 indices;
                QueueFamilyIndices2 target;

                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

                int i = 0;
                cBool selected = false;

                for (const auto &queueFamily : queueFamilies)
                {
                    // 그래픽스와 컴퓨트 큐 지원 확인
                    PRINT_TO_LOGGER("QueueFamily %d\n", i);
                    PRINT_TO_LOGGER("QueueFamily queueCount: %d\n", queueFamily.queueCount);
                    PRINT_TO_LOGGER("QueueFamily queueFlags: %d\n", queueFamily.queueFlags);
                    PRINT_TO_LOGGER("QueueFamily timestampValidBits: %d\n", queueFamily.timestampValidBits);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.width: %d\n", queueFamily.minImageTransferGranularity.width);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.height: %d\n", queueFamily.minImageTransferGranularity.height);
                    PRINT_TO_LOGGER("QueueFamily minImageTransferGranularity.depth: %d\n", queueFamily.minImageTransferGranularity.depth);

                    // 그래픽스 큐 지원 확인
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                        PRINT_TO_LOGGER("VK_QUEUE_GRAPHICS_BIT is supported\n");
                        if (!indices.graphicFamilyHasValue)
                        {
                            indices.setGraphicFamily(i);
                        }
                    }

                    // 컴퓨트 큐 지원 확인
                    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                    {
                        PRINT_TO_LOGGER("VK_QUEUE_COMPUTE_BIT is supported\n");
                        if (!indices.computerFamilyHasValue)
                        {
                            indices.setComputerFamily(i);
                        }
                    }

                    // 트랜스퍼 큐 지원 확인
                    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    {
                        PRINT_TO_LOGGER("VK_QUEUE_TRANSFER_BIT is supported\n");
                        if (!indices.transferFamilyHasValue)
                        {
                            indices.setTransferFamily(i);
                        }
                    }

                    if (indices.isComplete())
                    {

                        if (!selected)
                        {
                            indices.queueFamilyProperties = queueFamily;
                            target = indices;
                            selected = true;

                            PRINT_TO_LOGGER("------------------ select Queuefamily index: %d ------------------\n", i);
                        }
                    }

                    // Surface가 없으므로 Present 큐 확인은 생략
                    indices.reset();
                    i++;
                }

                return target;
            }

        } // namespace device
    } // namespace helper
} // namespace vkengine