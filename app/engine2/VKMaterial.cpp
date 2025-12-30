#include "VKMaterial.h"

#include <fstream>

namespace vkengine {

void VKMaterial::loadFromCache(const cString& cachePath)
{
    std::ifstream stream(cachePath, std::ios::binary);
    if (!stream.is_open()) {
        // Cache file doesn't exist or cannot be opened
        return;
    }

    try {
        // Read file format version for future compatibility
        uint32_t fileVersion;
        stream.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (!stream.good() || fileVersion != 1) {
            return; // Unsupported version or read error
        }

        // Read material name
        uint32_t nameLength;
        stream.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        if (!stream.good())
            return;

        if (nameLength > 0) {
            name.resize(nameLength);
            stream.read(&name[0], nameLength);
            if (!stream.good())
                return;
        }

        // Read material properties
        stream.read(reinterpret_cast<char*>(&ubo.emissiveFactor), sizeof(ubo.emissiveFactor));
        stream.read(reinterpret_cast<char*>(&ubo.baseColorFactor), sizeof(ubo.baseColorFactor));
        stream.read(reinterpret_cast<char*>(&ubo.roughness), sizeof(ubo.roughness));
        stream.read(reinterpret_cast<char*>(&ubo.transparencyFactor),
                    sizeof(ubo.transparencyFactor));
        stream.read(reinterpret_cast<char*>(&ubo.discardAlpha), sizeof(ubo.discardAlpha));
        stream.read(reinterpret_cast<char*>(&ubo.metallicFactor), sizeof(ubo.metallicFactor));

        // Read texture indices
        stream.read(reinterpret_cast<char*>(&ubo.baseColorTextureIndex),
                    sizeof(ubo.baseColorTextureIndex));
        stream.read(reinterpret_cast<char*>(&ubo.emissiveTextureIndex),
                    sizeof(ubo.emissiveTextureIndex));
        stream.read(reinterpret_cast<char*>(&ubo.normalTextureIndex),
                    sizeof(ubo.normalTextureIndex));
        stream.read(reinterpret_cast<char*>(&ubo.opacityTextureIndex),
                    sizeof(ubo.opacityTextureIndex));
        stream.read(reinterpret_cast<char*>(&ubo.metallicRoughnessTextureIndex),
                    sizeof(ubo.metallicRoughnessTextureIndex));
        stream.read(reinterpret_cast<char*>(&ubo.occlusionTextureIndex),
                    sizeof(ubo.occlusionTextureIndex));

        // Read flags
        stream.read(reinterpret_cast<char*>(&flags), sizeof(flags));

    } catch (...) {
        // If any error occurs, silently ignore and continue with default values
    }
}

void VKMaterial::writeToCache(const cString& cachePath)
{
    std::ofstream stream(cachePath, std::ios::binary);
    if (!stream.is_open()) {
        return; // Cannot create cache file
    }

    try {
        // Write file format version for future compatibility
        const uint32_t fileVersion = 1;
        stream.write(reinterpret_cast<const char*>(&fileVersion), sizeof(fileVersion));

        // Write material name
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        stream.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        if (nameLength > 0) {
            stream.write(name.c_str(), nameLength);
        }

        // Write material properties
        stream.write(reinterpret_cast<const char*>(&ubo.emissiveFactor),
                     sizeof(ubo.emissiveFactor));
        stream.write(reinterpret_cast<const char*>(&ubo.baseColorFactor),
                     sizeof(ubo.baseColorFactor));
        stream.write(reinterpret_cast<const char*>(&ubo.roughness), sizeof(ubo.roughness));
        stream.write(reinterpret_cast<const char*>(&ubo.transparencyFactor),
                     sizeof(ubo.transparencyFactor));
        stream.write(reinterpret_cast<const char*>(&ubo.discardAlpha),
                     sizeof(ubo.discardAlpha));
        stream.write(reinterpret_cast<const char*>(&ubo.metallicFactor),
                     sizeof(ubo.metallicFactor));

        // Write texture indices
        stream.write(reinterpret_cast<const char*>(&ubo.baseColorTextureIndex),
                     sizeof(ubo.baseColorTextureIndex));
        stream.write(reinterpret_cast<const char*>(&ubo.emissiveTextureIndex),
                     sizeof(ubo.emissiveTextureIndex));
        stream.write(reinterpret_cast<const char*>(&ubo.normalTextureIndex),
                     sizeof(ubo.normalTextureIndex));
        stream.write(reinterpret_cast<const char*>(&ubo.opacityTextureIndex),
                     sizeof(ubo.opacityTextureIndex));
        stream.write(reinterpret_cast<const char*>(&ubo.metallicRoughnessTextureIndex),
                     sizeof(ubo.metallicRoughnessTextureIndex));
        stream.write(reinterpret_cast<const char*>(&ubo.occlusionTextureIndex),
                     sizeof(ubo.occlusionTextureIndex));

        // Write flags
        stream.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

    } catch (...) {
        // If any error occurs, silently ignore
    }
}

} // namespace vkengine
