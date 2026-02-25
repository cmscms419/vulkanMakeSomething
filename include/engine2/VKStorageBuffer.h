#ifndef INCLUDE_VK_STOREGE_BUFFER_H_
#define INCLUDE_VK_STOREGE_BUFFER_H_

#include "VKContext.h"
#include "VKCommadBufferHander.h"

namespace vkengine {

    class VKStorgeBuffer {

    public:
        VKStorgeBuffer(VKcontext& ctx);
        ~VKStorgeBuffer();
        void cleanup();
        void create(VkDeviceSize size, VkBufferUsageFlags additionalUsage = 0);
        void* map();
        void unmap();
        void copyData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo getDescriptorInfo() const;

    private:
        VKcontext& ctx;
        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDeviceSize size;
        void* mapped;
        bool hostVisible;
    };

}

#endif // !
