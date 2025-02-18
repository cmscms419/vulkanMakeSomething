#ifndef INCLUDE_VULKANDEVICE_H_
#define INCLUDE_VULKANDEVICE_H_

#include "../_common.h"

namespace vkcms {

	struct VKDevice_
	{
		VkPhysicalDevice VKphysicalDevice;                  // 물리 디바이스 -> GPU Physical Handle
		VkDevice VKdevice;                                  // 논리 디바이스 -> GPU Logical Handle
	};
}


#endif // INCLUDE_VULKANDEVICE_H_