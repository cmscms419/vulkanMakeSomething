#ifndef INCLUDE_VULKANDEVICE_H_
#define INCLUDE_VULKANDEVICE_H_

#include "../_common.h"

namespace vkcms {

	struct VKDevice_
	{
		VkPhysicalDevice VKphysicalDevice;                  // ���� ����̽� -> GPU Physical Handle
		VkDevice VKdevice;                                  // �� ����̽� -> GPU Logical Handle
	};
}


#endif // INCLUDE_VULKANDEVICE_H_