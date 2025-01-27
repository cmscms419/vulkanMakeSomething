#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <map>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>

#define WIDTH  800
#define HEIGHT  600

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG_
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices {
    VkQueueFamilyProperties queueFamilyProperties = {};

    // optional은 값이 있을 수도 있고 없을 수도 있다.
    // C++14에서는 사용할 수 없다.
    uint32_t graphicsFamily = 0;        // 그래픽스 큐 패밀리 인덱스 (그래픽스 명령을 처리하는 큐)
    uint32_t presentFamily = 0;         // 프레젠트 큐 패밀리 인덱스 (윈도우 시스템과 Vulkan을 연결하는 인터페이스)

    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;

    void setGraphicsFamily(uint32_t index) {
        graphicsFamily = index;
        graphicsFamilyHasValue = true;
    }
    void setPresentFamily(uint32_t index) {
        presentFamily = index;
        presentFamilyHasValue = true;
    }
    uint32_t getGraphicsQueueFamilyIndex() {
        uint32_t target = -1;

        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = graphicsFamily;
        }

        return target;
    }
    uint32_t getPresentQueueFamilyIndex() {
        uint32_t target = -1;
        if (queueFamilyProperties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
        {
            target = presentFamily;
        }
        return target;
    }
    bool isComplete() {
        return this->graphicsFamilyHasValue && this->presentFamilyHasValue;
    }
    void reset() {
        this->graphicsFamily = 0;
        this->presentFamily = 0;
        this->graphicsFamilyHasValue = false;
        this->presentFamilyHasValue = false;
        this->queueFamilyProperties = {};
    }
};