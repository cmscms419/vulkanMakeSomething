#ifndef INCLUDE_SOURCE_DEBUGFUNCTION_H
#define INCLUDE_SOURCE_DEBUGFUNCTION_H

#include "../_common.h"

namespace vkutil {

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    bool checkValidationLayerSupport();
    
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

}

#endif // INCLUDE_SOURCE_DEBUGFUNCTION_H