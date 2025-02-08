#include "cms491_common.h"

namespace vkutil {
    namespace helper
    {
        std::vector<char> readFile(const std::string& filename) {

            // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // 파일을 열 수 없는 경우 예외를 발생시킵니다.
            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
            std::vector<char> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
            file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
            file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
            file.close();                           // 파일을 닫습니다.

            return buffer;
        }

        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
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

