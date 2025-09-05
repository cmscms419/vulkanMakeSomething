#ifndef INCLUDE_VK_CONTEXT_H_
#define INCLUDE_VK_CONTEXT_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKdevice2.h"
#include "VKDescriptorManager2.h"
#include "Debug.h"

namespace vkengine {
    class VkContext
    {
    public:
        VkContext(const std::vector<const char*>& requiredInstanceExtensions, cBool useSwapchain);
        ~VkContext();

        void cleanup();
        virtual cBool createInstance(std::vector<const char*> requiredInstanceExtensions);  // �ν��Ͻ� ����
        virtual cBool createPysicalDevice();                                                // ����̽�(logical, pysical)
        virtual cBool createLogicalDevice(cBool useSwapchain);                              // ���� ����̽� ����
        virtual cBool createCommandPools();                                                 // Ŀ�ǵ� Ǯ ���� (grapic, compute, transfer)
        virtual cBool createQueues();                                                       // ť ���� (grapic, compute, transfer)
        virtual cBool createPipelineCache();                                                // ���������� ĳ�� ����
        virtual cBool createDepthStencilFormat();                                           // ���� ���ٽ� ���� ����

        // ����
        cBool checkValidationLayerSupport(const cChar* str); // ���� ���̾� ���� Ȯ��

    private:
        VkInstance VKinstance{};                              // Vulkan �ν��Ͻ� -> Vulkan API�� ����ϱ� ���� �ν��Ͻ�
        VKdeviceHandler2 VKdevice;                            // ����̽� -> GPU Logical,Physical struct Handle
        VkPipelineCache VKpipelineCache{ VK_NULL_HANDLE };    // ���������� ĳ�� -> ���������� ĳ�ø� ����
        depthStencill VKdepthStencill{};                      // ���� ���ٽ� -> ���� ���ٽ� �̹����� �޸�
        DescriptorManager2 descriptorManager2;               // ��ũ���� �Ŵ��� -> ��ũ���� ��Ʈ ���̾ƿ��� ��ũ���� Ǯ ����
    };
}

#endif // !INCLUDE_VK_CONTEXT_H_
