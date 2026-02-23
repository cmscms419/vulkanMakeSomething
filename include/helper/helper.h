#ifndef INCLUDE_SOURCE_HELPER_H_
#define INCLUDE_SOURCE_HELPER_H_

#include <set>
#include <fstream>

#include "common.h"
#include "macros.h"
#include "log.h"
#include "vkdevice.h"

#include "helperDevice.h"
#include "helperCreate.h"
#include "helperFile.h"

// https://github.com/SaschaWillems/Vulkan에서 참고해서 함수 생성

namespace vkengine
{

    namespace helper
    {

        cString shaderStageFlagsToString(VkShaderStageFlags flags);

        cUint32_t getFormatSize(VkFormat format);

        VkFormat getVkFormatFromSpvReflectFormat(SpvReflectFormat format);

        void printReflectionInfo(const SpvReflectShaderModule &reflectModule);

        cString descriptorTypeToString(VkDescriptorType type);

        VkDescriptorType stringToDescriptorType(const cString &typeStr);

        const cChar *getShaderStageString(const SpvReflectShaderStageFlagBits &stage);

        const cChar *getDescriptorTypeString(SpvReflectDescriptorType type);





        

        // 최대 사용 가능한 샘플링 수를 반환하는 함수
        VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

        // setupCommandBuffer 나중에 추가
        // flushSetupCommands 나중에 추가

        // pyhsicalDeviceProperties를 가져오는 함수
        inline void getPyhsicalDeviceProperties(VkPhysicalDevice device)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            _PRINT_TO_CONSOLE_("Device Name: %s\n", deviceProperties.deviceName);
            _PRINT_TO_CONSOLE_("DeviceProperties.deviceType: %d\n", deviceProperties.deviceType);
        }

  
        

        // stencilComponent를 가지고 있는지 확인하는 함수
        cBool hasStencilComponent(VkFormat format);

        // 특정 데이터를 VkDeviceMemory 객체로 복사하는 함수
        template <typename T>
        inline void copyToDeviceMemory(VkDevice device, const T *src, VkDeviceMemory dst, VkDeviceSize size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0)
        {
            void *data;
            vkMapMemory(device, dst, offset, size, flags, &data);
            memcpy(data, src, (size_t)size);
            vkUnmapMemory(device, dst);
        }



    }

}

#endif // !INCLUDE_SOURCE_HELPER_H_