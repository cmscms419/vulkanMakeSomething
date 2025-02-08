#include "cms491_common.h"

namespace vkutil {
    namespace helper
    {
        std::vector<char> readFile(const std::string& filename) {

            // ���� ������ �̵��Ͽ� ���� ũ�⸦ �����ɴϴ�.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // ������ �� �� ���� ��� ���ܸ� �߻���ŵ�ϴ�.
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // ���� ũ�⸦ �̿��Ͽ� ���۸� �Ҵ��մϴ�.
            std::vector<char> buffer(fileSize);     // ���� �����͸� ������ �������� �̵��մϴ�.
            file.seekg(0);                          // ���� �����͸� ������ �������� �̵��մϴ�.
            file.read(buffer.data(), fileSize);     // ���� ������ ���ۿ� �о�ɴϴ�. -> ������ fileSize ũ�⸸ŭ �ѹ��� �о�´�.
            file.close();                           // ������ �ݽ��ϴ�.

            return buffer;
        }

        // ���� ����̽��� Ȯ�� ����� �����ϴ��� Ȯ���ϴ� �Լ�
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice VKphysicalDevice)
        {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(VKphysicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            throw std::runtime_error("failed to find suitable memory type!");
        }
    }
}

