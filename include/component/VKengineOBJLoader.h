#ifndef INCLUDE_VK_ENGINE_OBJ_LOADER_H_
#define INCLUDE_VK_ENGINE_OBJ_LOADER_H_

/*
* Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
*
* Copyright (C) 2018-2024 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "common.h"
#include "macros.h"
#include "struct.h"

#include <unordered_map>

namespace vkengine {
    namespace helper {
        namespace loadModel {
            namespace OBJ {
                void loadAsset(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
            }
        }
    }
}


#endif // INCLUDE_VK_LOAD_MODEL_H_