#ifndef INCLUDE_VK_LOAD_MODEL2_H_
#define INCLUDE_VK_LOAD_MODEL2_H_

/*
* Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
*
* Copyright (C) 2018-2024 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

/*
 * Note that this isn't a complete glTF loader and not all features of the glTF 2.0 spec are supported
 * For details on how glTF 2.0 works, see the official spec at https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
 *
 * If you are looking for a complete glTF implementation, check out https://github.com/SaschaWillems/Vulkan-glTF-PBR/
 */


#include "common.h"
#include "macros.h"
#include "struct.h"

#include "helper.h"
#include "VKdevice.h"

#include <unordered_map>

#include "tiny_gltf.h"

namespace vkengine {
    namespace helper {
		namespace vkglTF
		{
            void loadModelGLTF(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
		}
    }
}


#endif // INCLUDE_VK_LOAD_MODEL2_H_