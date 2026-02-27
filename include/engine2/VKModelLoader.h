#ifndef VK_MODELLOADER_INCLUDE_H_
#define VK_MODELLOADER_INCLUDE_H_

#include "common.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace vkengine {


class VKModel;
class VKModelNode;

class ModelLoader
{
  public:
    ModelLoader(VKModel& model);
    void loadFromModelFile(const string& modelFilename, bool readBistroObj);
    void loadFromCache(const string& cacheFilename);
    void writeToCache(const string& cacheFilename);

    void processNode(aiNode* node, const aiScene* scene, VKModelNode* parent = nullptr);
    void processMesh(aiMesh* mesh, const aiScene* scene, uint32_t meshIndex);
    void processMaterial(aiMaterial* material, const aiScene* scene, uint32_t materialIndex);
    void processMaterialBistro(aiMaterial* material, const aiScene* scene, uint32_t materialIndex);
    void processAnimations(const aiScene* scene);
    void processBones(const aiScene* scene);

    void debugWriteEmbeddedTextures() const;
    void optimizeMeshesBistro();
    void printVerticesAndIndices() const;
    void updateMatrices();

  private:
    VKModel& model;

    Assimp::Importer importer;
    string directory;
};

} // namespace vkengine

#endif // VK_MODELLOADER_INCLUDE_H_