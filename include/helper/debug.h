#ifndef INCLUDE_SOURCE_DEBUG_H
#define INCLUDE_SOURCE_DEBUG_H

#include "macros.h"
#include "common.h"

namespace vkengine {
    namespace debug {

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        void SetObjectName(VkDevice device, uint64_t objectHandle, VkObjectType objectType, const char* name);

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    }
}

#endif // INCLUDE_SOURCE_DEBUG_H