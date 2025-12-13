#ifndef INCLUDE_VKDESCRIPTOR_H_
#define INCLUDE_VKDESCRIPTOR_H_

#include "common.h"
#include "struct.h"
#include "macros.h"

#include "VKengine.h"
#include "VKbuffer.h"
#include "VKtexture.h"

namespace vkengine {

    struct VKDescriptor
    {

    public:
        VKDescriptor(VkDevice device = VK_NULL_HANDLE, uint16_t frames = 0) : logicaldevice(device), frames(frames) {};

        void destroyDescriptor();

        virtual void createDescriptorSetLayout(cBool useTexture) = 0; // 세이더가 지정된 위치의 리소스를 읽을 수 있게 해주는 인터페이스 제공
        virtual void createDescriptorPool(cBool useTexture) = 0;
        virtual void createDescriptorSets(cBool useTexture) = 0;
        virtual void updateDescriptorSets() = 0;
        virtual void BindDescriptorSets(VkCommandBuffer mainCommandBuffer, size_t currentFrame, uint16_t offset) = 0;

        void buildDescriptorSetLayout();
        void buildDescriptorPool();
        void buildDescriptorSets();

        virtual uint16_t getDescriptorCount() = 0;
        virtual VkPipelineLayout getPipelineLayout() = 0;

    private:
        void destroyDescriptorSetLayout();
        void destroyDescriptorSets();
        void destroyDescriptorPool();
        void destroyPipelineLayouts();

    protected:

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE }; // 파이프라인 레이아웃 -> 파이프라인 레이아웃을 생성

        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorPoolCreateInfo poolInfo{};

        std::vector<VkDescriptorSet> VKdescriptorSets{};
        VkDescriptorSetAllocateInfo allocInfo{};

        VkDevice logicaldevice{ VK_NULL_HANDLE };
        uint16_t frames = 0; // 프레임 수
    };

    // VKDescriptor2는 VKDescriptor와 달리 https://github.com/SaschaWillems/Vulkan를 보고 새로 만드는 Descriptor 구조체입니다.
    struct VKDescriptor2
    {
        VKDescriptor2(VkDevice device = VK_NULL_HANDLE, cUint16_t frames = 0) : logicaldevice(device), frames(frames) {};

        cUint16_t getDescriptorCount() { return static_cast<cUint16_t>(VKdescriptorSets.size()); }

        inline void createDescriptorPool() {
            _VK_CHECK_RESULT_(vkCreateDescriptorPool(this->logicaldevice, &poolInfo, nullptr, &this->VKdescriptorPool));
        }
        inline void createDescriptorSetLayout() {
            _VK_CHECK_RESULT_(vkCreateDescriptorSetLayout(this->logicaldevice, &layoutInfo, nullptr, &this->VKdescriptorSetLayout));
        }

        inline void createAllocateDescriptorSets() {
            _VK_CHECK_RESULT_(vkAllocateDescriptorSets(this->logicaldevice, &allocInfo, this->VKdescriptorSets.data()));
        }

        inline void createPipelineLayout() {
        
        }
        
        void BindDescriptorSets(VkCommandBuffer mainCommandBuffer, size_t currentFrame, uint16_t offset);

        //virtual void createDescriptorSets;
        //virtual void updateDescriptorSets;

        void destroyDescriptor();
        void destroyDescriptorSetLayout();
        void destroyDescriptorSets();
        void destroyDescriptorPool();
        void destroyPipelineLayouts();

        VkPipelineLayout VKpipelineLayout{ VK_NULL_HANDLE }; // 파이프라인 레이아웃 -> 파이프라인 레이아웃을 생성

        VkDescriptorSetLayout VKdescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};

        VkDescriptorPool VKdescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorPoolCreateInfo poolInfo{};

        std::vector<VkDescriptorSet> VKdescriptorSets{};
        VkDescriptorSetAllocateInfo allocInfo{};

        VkDevice logicaldevice{ VK_NULL_HANDLE };
        uint16_t frames = 0; // 프레임 수
    };

}

#endif // !INCLUDE_VKDESCRIPTOR_H_
