#define _CRT_SECURE_NO_WARNINGS

#include "VKModelLoader.h"
#include "VKModel.h"
#include "VKContext.h"

#include <chrono>
#include <filesystem>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glm/gtc/type_ptr.hpp> // For glm::make_mat4


using namespace vkengine::Log;

namespace vkengine {

    ModelLoader::ModelLoader(VKModel& model) : model(model)
    {
    }

    void ModelLoader::loadFromModelFile(const string& modelFilename, bool readBistroObj)
    {
        // Start timer for loading time measurement
        auto startTime = std::chrono::high_resolution_clock::now();

        // Generate cache file path based on model filename
        filesystem::path modelPath(modelFilename);
        string cacheFilename = modelPath.stem().string() + "_cache.bin";
        filesystem::path cachePath = modelPath.parent_path() / cacheFilename;

        // Check if cache file exists and is newer than the model file
        bool useCache = false;
        if (readBistroObj && filesystem::exists(cachePath)) {
            useCache = true;
        }

        // useCache = false; // 디버깅용 캐시 사용 중단

        // Try to load from cache first
        if (useCache) {
            loadFromCache(cachePath.string());

            // Check if cache loading was successful (non-empty model)
            if (!model.meshes.empty() && !model.materials.empty()) {
                // Load textures after successful cache load
                model.textures.reserve(model.textureFilenames.size());
                for (auto& filename : model.textureFilenames) {
                    string prefix = readBistroObj ? directory + "/LowRes/" : "";
                    model.textures.emplace_back(model.ctx);
                    model.textures.back().createTextureFromImage(
                        prefix + filename, false, model.textureSRgb[model.textures.size() - 1]);
                }

                // Calculate elapsed time
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

                PRINT_TO_LOGGER("Successfully loaded model from cache: {}", cachePath.string());
                PRINT_TO_LOGGER("  Meshes: {}", model.meshes.size());
                PRINT_TO_LOGGER("  Materials: {}", model.materials.size());
                PRINT_TO_LOGGER("  Loading time: {} ms", duration.count());
                return;
            }
            else {
                // Cache loading failed, clear any partially loaded data and fall back to model loading
                PRINT_TO_LOGGER("Cache loading failed, falling back to model file loading");
                model.cleanup();
            }
        }

        // Load from model file (original code)
        uint32_t importFlags = aiProcess_Triangulate;

        if (readBistroObj) {
            importFlags = 0 | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
                aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights |
                aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality |
                aiProcess_RemoveRedundantMaterials | aiProcess_FindDegenerates |
                aiProcess_FindInvalidData | aiProcess_GenUVCoords;
        }

        const aiScene* scene = importer.ReadFile(modelFilename, importFlags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            EXIT_TO_LOGGER("ERROR::ASSIMP: {}", importer.GetErrorString());
            return;
        }

        // Improved directory extraction using filesystem::path for cross-platform compatibility
        filesystem::path modelPath2(modelFilename);
        directory = modelPath2.parent_path().string();
        if (directory.empty()) {
            directory = "."; // Current directory if no path specified
        }

        PRINT_TO_LOGGER("VKModel directory: {}", directory);

        // Store global inverse transform for the VKModel class
        mat4 globalInverseTransform =
            glm::inverse(glm::make_mat4(&scene->mRootNode->mTransformation.a1));
        model.globalInverseTransform = globalInverseTransform;

        // Process materials first
        model.materials.resize(scene->mNumMaterials);
        for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
            if (readBistroObj) {
                processMaterialBistro(scene->mMaterials[i], scene, i);
            }
            else {
                processMaterial(scene->mMaterials[i], scene, i);
            }
        }

        // IMPORTANT: Process animations and bones BEFORE processing nodes/meshes
        // This ensures the VKAnimation system has the global bone mapping ready
        // when processMesh needs to assign global bone indices to vertices
        PRINT_TO_LOGGER("Processing animations and bones before mesh processing...");
        processAnimations(scene);
        processBones(scene);

        // AFTER animation processing, synchronize the global inverse transform
        if (model.animation) {
            model.animation->setGlobalInverseTransform(globalInverseTransform);
            PRINT_TO_LOGGER("Synchronized global inverse transform between VKModel and VKAnimation systems");
        }

        // Now process nodes and meshes - they can use the global bone indices
        processNode(scene->mRootNode, scene);
        model.calculateBoundingBox();

        // 안내: Bistro 모델은 파이썬 스크립트로 전처리한 저해상도 텍스쳐를 읽어들입니다.
        model.textures.reserve(model.textureFilenames.size());
        for (auto& filename : model.textureFilenames) {
            string prefix = readBistroObj ? directory + "/LowRes/" : directory + "/";
            model.textures.emplace_back(model.ctx);
            // Check if this is an embedded texture (indicated by * prefix)
            if (!filename.empty() && filename[0] == '*') {
                // Parse the texture index from the path (e.g., "*0" -> 0)
                int textureIndex = stoi(filename.substr(1));

                // Get the embedded texture from the scene
                const aiScene* scene = importer.GetScene();
                if (scene && textureIndex < static_cast<int>(scene->mNumTextures)) {
                    const aiTexture* aiTex = scene->mTextures[textureIndex];

                    int width, height, channels;
                    unsigned char* data = nullptr;

                    if (aiTex->mHeight == 0) {
                        // Compressed texture data (e.g., PNG, JPG)
                        data = stbi_load_from_memory(
                            reinterpret_cast<const unsigned char*>(aiTex->pcData), aiTex->mWidth,
                            &width, &height, &channels, STBI_rgb_alpha);
                    }
                    else {
                        // Uncompressed RGBA texture data
                        width = aiTex->mWidth;
                        height = aiTex->mHeight;
                        channels = 4;

                        // Convert aiTexel to RGBA8
                        size_t dataSize = width * height * 4;
                        data = static_cast<unsigned char*>(malloc(dataSize));

                        for (int i = 0; i < width * height; ++i) {
                            data[i * 4 + 0] = static_cast<unsigned char>(aiTex->pcData[i].r * 255);
                            data[i * 4 + 1] = static_cast<unsigned char>(aiTex->pcData[i].g * 255);
                            data[i * 4 + 2] = static_cast<unsigned char>(aiTex->pcData[i].b * 255);
                            data[i * 4 + 3] = static_cast<unsigned char>(aiTex->pcData[i].a * 255);
                        }
                    }

                    if (data) {
                        // Create texture directly from memory data
                        model.textures.back().createTextureFromPixelData(
                            data, width, height, 4, model.textureSRgb[model.textures.size() - 1]);

                        // Free memory
                        if (aiTex->mHeight == 0) {
                            stbi_image_free(data); // Free stbi allocated memory
                        }
                        else {
                            free(data); // Free manually allocated memory
                        }

                        PRINT_TO_LOGGER("Loaded embedded texture {} ({}x{}) with {} format", textureIndex,
                            width, height,
                            model.textureSRgb[model.textures.size() - 1] ? "sRGB" : "linear");
                    }
                    else {
                        PRINT_TO_LOGGER("WARNING: Failed to decode embedded texture {}", textureIndex);
                        if (aiTex->mHeight == 0) {
                            PRINT_TO_LOGGER("  Reason: {}", stbi_failure_reason());
                        }
                    }
                }
                else {
                    PRINT_TO_LOGGER("WARNING: Embedded texture index {} out of range (max: {})", textureIndex,
                        scene ? scene->mNumTextures : 0);
                }
            }
            else {
                // External texture file - use existing path logic

                string prefix = readBistroObj ? directory + "/LowRes/" : directory + "/";
                // textures.back().createTextureFromImage(prefix + filename, false,
                //                                         textureSRgb[textures.size() - 1]);

                // 안내:
                // - 캐릭터 fbx는 미리 추출한 텍스쳐 사용
                // - 같은 폴더에 있기 때문에 파일이름에서 폴더명 제거
                string shortFilename =
                    readBistroObj ? filename : filesystem::path(filename).filename().string();

                PRINT_TO_LOGGER("Texture filename: {}", prefix + shortFilename);

                model.textures.back().createTextureFromImage(
                    prefix + shortFilename, false, model.textureSRgb[model.textures.size() - 1]);
            }
        }

        // Calculate elapsed time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        PRINT_TO_LOGGER("Successfully loaded model: {}", modelFilename);
        PRINT_TO_LOGGER("  Meshes: {}", model.meshes.size());
        PRINT_TO_LOGGER("  Materials: {}", model.materials.size());
        PRINT_TO_LOGGER("  Loading time: {} ms", duration.count());

        if (readBistroObj && !useCache) {
            optimizeMeshesBistro();
            writeToCache(cachePath.string());
            PRINT_TO_LOGGER("VKModel cached to: {}", cachePath.string());
        }

        return;
    }

    void ModelLoader::loadFromCache(const string& cacheFilename)
    {
        std::ifstream stream(cacheFilename, std::ios::binary);
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

            // Read directory
            uint32_t dirLength;
            stream.read(reinterpret_cast<char*>(&dirLength), sizeof(dirLength));
            if (!stream.good())
                return;

            if (dirLength > 0) {
                directory.resize(dirLength);
                stream.read(&directory[0], dirLength);
                if (!stream.good())
                    return;
            }

            // Read global inverse transform
            stream.read(reinterpret_cast<char*>(&model.globalInverseTransform),
                sizeof(model.globalInverseTransform));
            if (!stream.good())
                return;

            // Read bounding box
            stream.read(reinterpret_cast<char*>(&model.boundingBoxMin),
                sizeof(model.boundingBoxMin));
            stream.read(reinterpret_cast<char*>(&model.boundingBoxMax),
                sizeof(model.boundingBoxMax));
            if (!stream.good())
                return;

            // Read texture filenames
            uint32_t textureCount;
            stream.read(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));
            if (!stream.good())
                return;

            model.textureFilenames.clear();
            model.textureSRgb.clear();
            model.textureFilenames.reserve(textureCount);
            model.textureSRgb.reserve(textureCount);

            for (uint32_t i = 0; i < textureCount; ++i) {
                uint32_t filenameLength;
                stream.read(reinterpret_cast<char*>(&filenameLength), sizeof(filenameLength));
                if (!stream.good())
                    return;

                string filename;
                if (filenameLength > 0) {
                    filename.resize(filenameLength);
                    stream.read(&filename[0], filenameLength);
                    if (!stream.good())
                        return;
                }
                model.textureFilenames.push_back(std::move(filename));

                bool sRGB;
                stream.read(reinterpret_cast<char*>(&sRGB), sizeof(sRGB));
                if (!stream.good())
                    return;
                model.textureSRgb.push_back(sRGB);
            }

            // Read meshes
            uint32_t meshCount;
            stream.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
            if (!stream.good())
                return;

            model.meshes.clear();
            model.reserveMeshes(meshCount);
            for (uint32_t i = 0; i < meshCount; ++i) {
                Mesh& mesh = model.addMesh();

                if (!mesh.readFromBinaryFileStream(stream)) {
                    return;
                }
            }

            // Read materials
            uint32_t materialCount;
            stream.read(reinterpret_cast<char*>(&materialCount), sizeof(materialCount));
            if (!stream.good())
                return;

            model.materials.clear();
            model.materials.resize(materialCount);
            for (uint32_t i = 0; i < materialCount; ++i) {
                model.materials[i].loadFromCache(
                    ""); // Use empty string since we're reading from stream

                // Read material data directly from our stream
                uint32_t materialVersion;
                stream.read(reinterpret_cast<char*>(&materialVersion), sizeof(materialVersion));
                if (!stream.good() || materialVersion != 1)
                    return;

                // Read material name
                uint32_t nameLength;
                stream.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
                if (!stream.good())
                    return;

                if (nameLength > 0) {
                    model.materials[i].name.resize(nameLength);
                    stream.read(&model.materials[i].name[0], nameLength);
                    if (!stream.good())
                        return;
                }

                // Read material properties
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.emissiveFactor),
                    sizeof(model.materials[i].ubo.emissiveFactor));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.baseColorFactor),
                    sizeof(model.materials[i].ubo.baseColorFactor));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.roughness),
                    sizeof(model.materials[i].ubo.roughness));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.transparencyFactor),
                    sizeof(model.materials[i].ubo.transparencyFactor));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.discardAlpha),
                    sizeof(model.materials[i].ubo.discardAlpha));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.metallicFactor),
                    sizeof(model.materials[i].ubo.metallicFactor));

                // Read texture indices
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.baseColorTextureIndex),
                    sizeof(model.materials[i].ubo.baseColorTextureIndex));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.emissiveTextureIndex),
                    sizeof(model.materials[i].ubo.emissiveTextureIndex));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.normalTextureIndex),
                    sizeof(model.materials[i].ubo.normalTextureIndex));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.opacityTextureIndex),
                    sizeof(model.materials[i].ubo.opacityTextureIndex));
                stream.read(
                    reinterpret_cast<char*>(&model.materials[i].ubo.metallicRoughnessTextureIndex),
                    sizeof(model.materials[i].ubo.metallicRoughnessTextureIndex));
                stream.read(reinterpret_cast<char*>(&model.materials[i].ubo.occlusionTextureIndex),
                    sizeof(model.materials[i].ubo.occlusionTextureIndex));

                // Read flags
                stream.read(reinterpret_cast<char*>(&model.materials[i].flags),
                    sizeof(model.materials[i].flags));
                if (!stream.good())
                    return;
            }

            // Initialize an empty root node - the hierarchical structure
            // is complex and might not be worth caching for performance gains
            model.rootNode = make_unique<VKModelNode>();
            model.rootNode->name = "Root";

            // Textures will need to be reloaded from files since they contain
            // device-specific Vulkan resources that can't be serialized
            model.textures.clear();

        }
        catch (...) {
            // If any error occurs, clear data and continue with empty model
            model.meshes.clear();
            model.materials.clear();
            model.textureFilenames.clear();
            model.textureSRgb.clear();
            model.textures.clear();
        }
    }

    void ModelLoader::writeToCache(const string& cacheFilename)
    {
        std::ofstream stream(cacheFilename, std::ios::binary);
        if (!stream.is_open()) {
            return; // Cannot create cache file
        }

        try {
            // Write file format version for future compatibility
            const uint32_t fileVersion = 1;
            stream.write(reinterpret_cast<const char*>(&fileVersion), sizeof(fileVersion));

            // Write directory
            uint32_t dirLength = static_cast<uint32_t>(directory.length());
            stream.write(reinterpret_cast<const char*>(&dirLength), sizeof(dirLength));
            if (dirLength > 0) {
                stream.write(directory.c_str(), dirLength);
            }

            // Write global inverse transform
            stream.write(reinterpret_cast<const char*>(&model.globalInverseTransform),
                sizeof(model.globalInverseTransform));

            // Write bounding box
            stream.write(reinterpret_cast<const char*>(&model.boundingBoxMin),
                sizeof(model.boundingBoxMin));
            stream.write(reinterpret_cast<const char*>(&model.boundingBoxMax),
                sizeof(model.boundingBoxMax));

            // Write texture filenames and sRGB flags
            uint32_t textureCount = static_cast<uint32_t>(model.textureFilenames.size());
            stream.write(reinterpret_cast<const char*>(&textureCount), sizeof(textureCount));

            for (uint32_t i = 0; i < textureCount; ++i) {
                uint32_t filenameLength = static_cast<uint32_t>(model.textureFilenames[i].length());
                stream.write(reinterpret_cast<const char*>(&filenameLength), sizeof(filenameLength));
                if (filenameLength > 0) {
                    stream.write(model.textureFilenames[i].c_str(), filenameLength);
                }

                bool sRGB = (i < model.textureSRgb.size()) ? model.textureSRgb[i] : false;
                stream.write(reinterpret_cast<const char*>(&sRGB), sizeof(sRGB));
            }

            // Write meshes
            uint32_t meshCount = static_cast<uint32_t>(model.meshes.size());
            stream.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

            for (const auto& mesh : model.meshes) {
                if (!mesh.writeToBinaryFileStream(stream)) {
                    return;
                }
            }

            // Write materials
            uint32_t materialCount = static_cast<uint32_t>(model.materials.size());
            stream.write(reinterpret_cast<const char*>(&materialCount), sizeof(materialCount));

            for (const auto& material : model.materials) {
                // Write material data directly to our stream (similar to VKMaterial::writeToCache)
                const uint32_t materialVersion = 1;
                stream.write(reinterpret_cast<const char*>(&materialVersion), sizeof(materialVersion));

                // Write material name
                uint32_t nameLength = static_cast<uint32_t>(material.name.length());
                stream.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
                if (nameLength > 0) {
                    stream.write(material.name.c_str(), nameLength);
                }

                // Write material properties
                stream.write(reinterpret_cast<const char*>(&material.ubo.emissiveFactor),
                    sizeof(material.ubo.emissiveFactor));
                stream.write(reinterpret_cast<const char*>(&material.ubo.baseColorFactor),
                    sizeof(material.ubo.baseColorFactor));
                stream.write(reinterpret_cast<const char*>(&material.ubo.roughness),
                    sizeof(material.ubo.roughness));
                stream.write(reinterpret_cast<const char*>(&material.ubo.transparencyFactor),
                    sizeof(material.ubo.transparencyFactor));
                stream.write(reinterpret_cast<const char*>(&material.ubo.discardAlpha),
                    sizeof(material.ubo.discardAlpha));
                stream.write(reinterpret_cast<const char*>(&material.ubo.metallicFactor),
                    sizeof(material.ubo.metallicFactor));

                // Write texture indices
                stream.write(reinterpret_cast<const char*>(&material.ubo.baseColorTextureIndex),
                    sizeof(material.ubo.baseColorTextureIndex));
                stream.write(reinterpret_cast<const char*>(&material.ubo.emissiveTextureIndex),
                    sizeof(material.ubo.emissiveTextureIndex));
                stream.write(reinterpret_cast<const char*>(&material.ubo.normalTextureIndex),
                    sizeof(material.ubo.normalTextureIndex));
                stream.write(reinterpret_cast<const char*>(&material.ubo.opacityTextureIndex),
                    sizeof(material.ubo.opacityTextureIndex));
                stream.write(
                    reinterpret_cast<const char*>(&material.ubo.metallicRoughnessTextureIndex),
                    sizeof(material.ubo.metallicRoughnessTextureIndex));
                stream.write(reinterpret_cast<const char*>(&material.ubo.occlusionTextureIndex),
                    sizeof(material.ubo.occlusionTextureIndex));

                // Write flags
                stream.write(reinterpret_cast<const char*>(&material.flags), sizeof(material.flags));
            }

        }
        catch (...) {
            // If any error occurs, silently ignore
        }
    }

    void ModelLoader::processNode(aiNode* node, const aiScene* scene, VKModelNode* parent)
    {
        // Create new VKModelNode
        unique_ptr<VKModelNode> modelNode = make_unique<VKModelNode>();
        modelNode->name = string(node->mName.C_Str());
        modelNode->parent = parent;

        modelNode->localMatrix = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

        // Extract transformation components if needed
        aiVector3D scaling, position;
        aiQuaternion rotation;
        node->mTransformation.Decompose(scaling, rotation, position);

        modelNode->translation = vec3(position.x, position.y, position.z);
        modelNode->rotation = quat(rotation.w, rotation.x, rotation.y, rotation.z);
        modelNode->scale = vec3(scaling.x, scaling.y, scaling.z);

        // Process all meshes in this node
        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            uint32_t meshIndex = node->mMeshes[i];

            // Ensure meshes vector is large enough
            if (meshIndex >= model.Meshes().size()) {
                model.reserveMeshes(meshIndex + 1);
                while (model.Meshes().size() <= meshIndex) {
                    model.addMesh();
                }
            }

            // Process the mesh if it hasn't been processed yet
            if (model.meshes[meshIndex].vertices.empty()) {
                processMesh(scene->mMeshes[meshIndex], scene, meshIndex);
            }

            modelNode->meshIndices.push_back(meshIndex);
        }

        // Store pointer to current node for child processing
        VKModelNode* currentNode = modelNode.get();

        // Add to parent or root
        if (parent) {
            parent->children.push_back(move(modelNode));
        }
        else {
            model.rootNode = move(modelNode);
        }

        // Process children
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene, currentNode);
        }
    }

    void ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, uint32_t meshIndex)
    {
        auto& meshes = model.meshes;

        // Ensure meshes vector is large enough
        if (meshIndex >= meshes.size()) {
            meshes.resize(meshIndex + 1);
        }

        Mesh& currentMesh = meshes[meshIndex];

        currentMesh.name = string(mesh->mName.C_Str());

        // Process vertices with UV validation
        currentMesh.vertices.reserve(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex2 vertex;

            vertex.pos = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            // Normal
            if (mesh->HasNormals()) {
                vertex.normal = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else {
                vertex.normal = vec3(0.0f, 1.0f, 0.0f);
            }

            // Texture coordinates with validation
            if (mesh->mTextureCoords[0]) {
                vertex.texCoord = vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
                vertex.texCoord.y = 1.0f - vertex.texCoord.y; // y fliped
            }
            else {
                vertex.texCoord = vec2(0.0f, 0.0f);
                currentMesh.noTextureCoords = true;
            }

            // Tangent
            if (mesh->HasTangentsAndBitangents()) {
                vertex.inTangent = vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                vertex.Bitangent =
                    vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            }
            else {
                vertex.inTangent = vec3(1.0f, 0.0f, 0.0f);
                vertex.Bitangent = vec3(0.0f, 0.0f, 1.0f);
            }

            currentMesh.vertices.push_back(vertex);
        }

        // Process indices
        currentMesh.indices.reserve(mesh->mNumFaces * 3);
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            if (mesh->mFaces[i].mNumIndices != 3)
                continue;
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                currentMesh.indices.push_back(face.mIndices[j]);
            }
        }

        // Set material index
        currentMesh.materialIndex = mesh->mMaterialIndex;

        // Calculate bounds immediately after vertices are populated
        currentMesh.calculateBounds();

        // Process bone weights and indices for skeletal animation
        if (mesh->HasBones()) {
            PRINT_TO_LOGGER("Processing {} bones for mesh '{}'", mesh->mNumBones, mesh->mName.C_Str());

            // First pass: collect bone weights for each vertex using GLOBAL bone indices
            for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                const aiBone* bone = mesh->mBones[boneIndex];
                string boneName = bone->mName.C_Str();

                // Get the GLOBAL bone index from the VKAnimation system
                int globalBoneIndex = -1;
                if (model.animation) {
                    globalBoneIndex =
                        model.animation->getGlobalBoneIndex(boneName); // FIXED: Call as method
                }

                if (globalBoneIndex == -1) {
                    PRINT_TO_LOGGER(
                        "WARNING: Bone '{}' not found in global bone mapping, using local index {}",
                        boneName, boneIndex);
                    globalBoneIndex = static_cast<int>(boneIndex); // Fallback to local index
                }

                // Add bone weights to vertices using the global bone index
                for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                    const aiVertexWeight& weight = bone->mWeights[weightIndex];
                    uint32_t vertexId = weight.mVertexId;

                    if (vertexId < currentMesh.vertices.size()) {
                        // Use the GLOBAL bone index instead of local mesh bone index
                        currentMesh.vertices[vertexId].addBoneData(
                            static_cast<uint32_t>(globalBoneIndex), weight.mWeight);
                    }
                }
            }

            // Second pass: normalize bone weights
            for (auto& vertex : currentMesh.vertices) {
                vertex.normalizeBoneWeights();
            }
        }

        // print("Processed mesh {} with {} vertices and {} indices\n", meshIndex,
        //       currentMesh.vertices.size(), currentMesh.indices.size());
    }

    void ModelLoader::processMaterial(aiMaterial* material, const aiScene* scene,
        uint32_t materialIndex)
    {
        VKMaterial& mat = model.materials[materialIndex];

        // Base color
        aiColor3D color;
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            mat.ubo.baseColorFactor = vec4(color.r, color.g, color.b, 1.0f);
        }

        // Metallic factor
        float metallic;
        if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
            mat.ubo.metallicFactor = metallic;
        }

        // Roughness factor
        float roughness;
        if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            mat.ubo.roughness = roughness;
        }

        // Emissive factor
        aiColor3D emissive;
        if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
            mat.ubo.emissiveFactor = vec4(emissive.r, emissive.g, emissive.b, 1.0f);
        }

        auto getTextureIndex = [this](const string& textureName, bool sRGB) -> int {
            auto it = std::find(model.textureFilenames.begin(), model.textureFilenames.end(),
                textureName);
            if (it != model.textureFilenames.end()) {
                return static_cast<int>(std::distance(model.textureFilenames.begin(), it));
            }
            else {
                model.textureFilenames.push_back(textureName);
                model.textureSRgb.push_back(sRGB); // Store sRGB flag
                assert(model.textureFilenames.size() == model.textureSRgb.size());
                return static_cast<int>(model.textureFilenames.size() - 1);
            }
        };

        // Load textures
        aiString texturePath;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            mat.ubo.baseColorTextureIndex = getTextureIndex(texturePath.C_Str(), true);
        }

        if (material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texturePath) ==
            AI_SUCCESS) {
            mat.ubo.metallicRoughnessTextureIndex = getTextureIndex(texturePath.C_Str(), false);
        }
        else if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
            mat.ubo.metallicRoughnessTextureIndex = getTextureIndex(texturePath.C_Str(), false);
            // for Bistro model
        }

        if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
            mat.ubo.normalTextureIndex = getTextureIndex(texturePath.C_Str(), false);
        }
        if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &texturePath) == AI_SUCCESS) {
            mat.ubo.occlusionTextureIndex = getTextureIndex(texturePath.C_Str(), false);
        }
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
            mat.ubo.emissiveTextureIndex = getTextureIndex(texturePath.C_Str(), false);
        }

        PRINT_TO_LOGGER("Processed materialIndex {}:\n", materialIndex);
        PRINT_TO_LOGGER("  Base color: %f, %f, %f, %f\n", mat.ubo.baseColorFactor.x, mat.ubo.baseColorFactor.y, mat.ubo.baseColorFactor.z, mat.ubo.baseColorFactor.w);
        PRINT_TO_LOGGER("  Metallic factor: %f \n", mat.ubo.metallicFactor);
        PRINT_TO_LOGGER("  Roughness factor: %f \n", mat.ubo.roughness);
        PRINT_TO_LOGGER("  Emissive factor: %f, %f, %f, %f\n", mat.ubo.emissiveFactor.x, mat.ubo.emissiveFactor.y, mat.ubo.emissiveFactor.z, mat.ubo.emissiveFactor.w);
        PRINT_TO_LOGGER("  Base color texture: {}\n", mat.ubo.baseColorTextureIndex != -1 ? "Loaded" : "None");
        PRINT_TO_LOGGER("  MetallicRoughness texture: {}\n", mat.ubo.metallicRoughnessTextureIndex != -1 ? "Loaded" : "None"); 
        PRINT_TO_LOGGER("  Normal texture: {}\n", mat.ubo.normalTextureIndex != -1 ? "Loaded" : "None");
        PRINT_TO_LOGGER("  Occlusion texture : {}\n", mat.ubo.occlusionTextureIndex != -1 ? "Loaded" : "None");
        PRINT_TO_LOGGER("  Emissive texture : {}\n", mat.ubo.emissiveTextureIndex != -1 ? "Loaded" : "None");
    }

    void ModelLoader::processMaterialBistro(aiMaterial* aiMat, const aiScene* scene,
        uint32_t materialIndex)
    {
        VKMaterial& mat = model.materials[materialIndex];

        // Read parameters
        {
            aiColor4D Color;

            if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_AMBIENT, &Color) == AI_SUCCESS) {
                mat.ubo.emissiveFactor = { Color.r, Color.g, Color.b, Color.a };
                if (mat.ubo.emissiveFactor.w > 1.0f)
                    mat.ubo.emissiveFactor.w = 1.0f;
            }
            if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_DIFFUSE, &Color) == AI_SUCCESS) {
                mat.ubo.baseColorFactor = { Color.r, Color.g, Color.b, Color.a };
                if (mat.ubo.baseColorFactor.w > 1.0f)
                    mat.ubo.baseColorFactor.w = 1.0f;
            }
            if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_EMISSIVE, &Color) == AI_SUCCESS) {
                mat.ubo.emissiveFactor += vec4(Color.r, Color.g, Color.b, Color.a);
                if (mat.ubo.emissiveFactor.w > 1.0f)
                    mat.ubo.emissiveFactor.w = 1.0f;
            }

            const float opaquenessThreshold = 0.05f;
            float Opacity = 1.0f;

            if (aiGetMaterialFloat(aiMat, AI_MATKEY_OPACITY, &Opacity) == AI_SUCCESS) {
                mat.ubo.transparencyFactor = glm::clamp(1.0f - Opacity, 0.0f, 1.0f);
                if (mat.ubo.transparencyFactor >= 1.0f - opaquenessThreshold)
                    mat.ubo.transparencyFactor = 0.0f;
            }

            if (aiGetMaterialColor(aiMat, AI_MATKEY_COLOR_TRANSPARENT, &Color) == AI_SUCCESS) {
                const float Opacity = std::max(std::max(Color.r, Color.g), Color.b);
                mat.ubo.transparencyFactor = glm::clamp(Opacity, 0.0f, 1.0f);
                if (mat.ubo.transparencyFactor >= 1.0f - opaquenessThreshold)
                    mat.ubo.transparencyFactor = 0.0f;
                mat.ubo.discardAlpha = 0.5f;
            }

            float tmp = 1.0f;
            if (aiGetMaterialFloat(aiMat, AI_MATKEY_METALLIC_FACTOR, &tmp) == AI_SUCCESS)
                mat.ubo.metallicFactor = tmp;

            if (aiGetMaterialFloat(aiMat, AI_MATKEY_ROUGHNESS_FACTOR, &tmp) == AI_SUCCESS)
                mat.ubo.roughness = tmp;
        }

        // Load textures
        {
            // 안내: Bistro 모델의 텍스쳐 경로에는 앞에 "..\\"가 덧붙어 있어서 나중에 제거합니다.
            auto getTextureIndex = [this](string textureName, bool sRGB) -> int {
                filesystem::path fullPath("dummy/" + string(textureName));
                textureName = fullPath.lexically_normal().string();
                // PRINT_TO_LOGGER("Texture filename: {}", textureName);
                auto it = std::find(model.textureFilenames.begin(), model.textureFilenames.end(),
                    textureName);
                if (it != model.textureFilenames.end()) {
                    return static_cast<int>(std::distance(model.textureFilenames.begin(), it));
                }
                else {
                    model.textureFilenames.push_back(textureName);
                    model.textureSRgb.push_back(sRGB); // Store sRGB flag
                    assert(model.textureFilenames.size() == model.textureSRgb.size());
                    return static_cast<int>(model.textureFilenames.size() - 1);
                }
            };

            aiString texturePath;

            if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
                mat.ubo.emissiveTextureIndex = getTextureIndex(texturePath.C_Str(), false);
            }

            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                mat.ubo.baseColorTextureIndex = getTextureIndex(texturePath.C_Str(), true);
                const std::string albedoMap = std::string(texturePath.C_Str());
                if (albedoMap.find("grey_30") != albedoMap.npos)
                    mat.flags |= VKMaterial::sTransparent;
            }

            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
                mat.ubo.normalTextureIndex = getTextureIndex(texturePath.C_Str(), false);
            }
            else if (aiMat->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
                mat.ubo.normalTextureIndex = getTextureIndex(texturePath.C_Str(), false);
            }

            if (aiMat->GetTexture(aiTextureType_OPACITY, 0, &texturePath) == AI_SUCCESS) {
                mat.ubo.opacityTextureIndex = getTextureIndex(texturePath.C_Str(), false);
                mat.ubo.discardAlpha = 0.5f;
            }

            // TODO: 필요하면 PBR (Metallic-Roughness) 텍스처도 추가로 로드합니다.
        }

        // 기타 경험적으로 필요한 것들
        {
            aiString Name;
            std::string materialName;
            if (aiGetMaterialString(aiMat, AI_MATKEY_NAME, &Name) == AI_SUCCESS) {
                materialName = Name.C_Str();
            }

            mat.name = materialName;

            auto name = [&materialName](const char* substr) -> bool {
                return materialName.find(substr) != std::string::npos;
            };
            if (name("MASTER_Glass_Clean") || name("MenuSign_02_Glass") || name("Vespa_Headlight")) {
                mat.ubo.discardAlpha = 0.75f;
                mat.ubo.transparencyFactor = 0.2f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("MASTER_Glass_Exterior") || name("MASTER_Focus_Glass")) {
                mat.ubo.discardAlpha = 0.75f;
                mat.ubo.transparencyFactor = 0.3f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("MASTER_Frosted_Glass") || name("MASTER_Interior_01_Frozen_Glass")) {
                mat.ubo.discardAlpha = 0.75f;
                mat.ubo.transparencyFactor = 0.2f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("Streetlight_Glass")) {
                mat.ubo.discardAlpha = 0.75f;
                mat.ubo.transparencyFactor = 0.15f;
                mat.ubo.baseColorTextureIndex = -1;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("Paris_LiquorBottle_01_Glass_Wine")) {
                mat.ubo.discardAlpha = 0.56f;
                mat.ubo.transparencyFactor = 0.35f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("_Caps") || name("_Labels")) {
                // not transparent
            }
            else if (name("Paris_LiquorBottle_02_Glass")) {
                mat.ubo.discardAlpha = 0.56f;
                mat.ubo.transparencyFactor = 0.1f;
            }
            else if (name("Bottle")) {
                mat.ubo.discardAlpha = 0.56f;
                mat.ubo.transparencyFactor = 0.2f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("Glass")) {
                mat.ubo.discardAlpha = 0.56f;
                mat.ubo.transparencyFactor = 0.1f;
                mat.flags |= VKMaterial::sTransparent;
            }
            else if (name("Metal")) {
                mat.ubo.metallicFactor = 1.0f;
                mat.ubo.roughness = 0.1f;
            }
        }
    }

    void ModelLoader::updateMatrices()
    {
        if (model.rootNode) {
            model.rootNode->updateWorldMatrix();
        }
    }

    void ModelLoader::printVerticesAndIndices() const
    {
        PRINT_TO_LOGGER("\nModel Vertices and Indices");
        PRINT_TO_LOGGER("  File: {}", directory);
        PRINT_TO_LOGGER("  Total meshes: {}", model.meshes.size());
        PRINT_TO_LOGGER("  Total materials: {}", model.materials.size());
        PRINT_TO_LOGGER("  VKModel bounding box: min({}, {}, {}), max({}, {}, {})", model.boundingBoxMin.x,
            model.boundingBoxMin.y, model.boundingBoxMin.z, model.boundingBoxMax.x,
            model.boundingBoxMax.y, model.boundingBoxMax.z);

        return;

        for (size_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx) {
            const Mesh& mesh = model.meshes[meshIdx];

            PRINT_TO_LOGGER("  Mesh {}: vertices = {}, indices = {}, material = {}", meshIdx,
                mesh.vertices.size(), mesh.materialIndex, mesh.indices.size(),
                mesh.materialIndex);
            PRINT_TO_LOGGER("  Mesh bounding box: min({}, {}, {}), max({}, {}, {})", mesh.minBounds.x,
                mesh.minBounds.y, mesh.minBounds.z, mesh.maxBounds.x, mesh.maxBounds.y,
                mesh.maxBounds.z);

            // Print vertices (limit to first 10 to avoid spam)
            // size_t maxVertices = std::min(mesh.vertices.size(), static_cast<size_t>(10));
            // print("Vertices (first {}):\n", maxVertices);
            // for (size_t i = 0; i < maxVertices; ++i) {
            //    const Vertex &v = mesh.vertices[i];
            //    print("  [{}] pos: ({}, {}, {})\n", i, v.position.x, v.position.y, v.position.z);
            //    print("       normal: ({}, {}, {})\n", v.normal.x, v.normal.y, v.normal.z);
            //    print("       texCoord: ({}, {})\n", v.texCoord.x, v.texCoord.y);
            //    print("       tangent: ({}, {}, {})\n", v.tangent.x, v.tangent.y, v.tangent.z);
            //    print("       bitangent: ({}, {}, {})\n", v.bitangent.x, v.bitangent.y,
            //    v.bitangent.z);
            //}

            // if (mesh.vertices.size() > maxVertices) {
            //     print("  ... and {} more vertices\n", mesh.vertices.size() - maxVertices);
            // }

            //// Print indices (limit to first 30 to avoid spam)
            // size_t maxIndices = std::min(mesh.indices.size(), static_cast<size_t>(30));
            // print("Indices (first {}):\n  ", maxIndices);
            // for (size_t i = 0; i < maxIndices; ++i) {
            //     print("{}", mesh.indices[i]);
            //     if (i < maxIndices - 1)
            //         print(", ");
            //     if ((i + 1) % 10 == 0)
            //         print("\n  "); // New line every 10 indices
            // }
            // print("\n");

            // if (mesh.indices.size() > maxIndices) {
            //     print("  ... and {} more indices\n", mesh.indices.size() - maxIndices);
            // }

            //// Print triangles formed by first few indices
            // print("First few triangles:\n");
            // size_t maxTriangles = std::min(mesh.indices.size() / 3, static_cast<size_t>(3));
            // for (size_t i = 0; i < maxTriangles; ++i) {
            //     size_t idx0 = mesh.indices[i * 3 + 0];
            //     size_t idx1 = mesh.indices[i * 3 + 1];
            //     size_t idx2 = mesh.indices[i * 3 + 2];

            //    print("  Triangle {}: indices [{}, {}, {}]\n", i, idx0, idx1, idx2);

            //    if (idx0 < mesh.vertices.size() && idx1 < mesh.vertices.size() &&
            //        idx2 < mesh.vertices.size()) {
            //        const Vertex &v0 = mesh.vertices[idx0];
            //        const Vertex &v1 = mesh.vertices[idx1];
            //        const Vertex &v2 = mesh.vertices[idx2];

            //        print("    v0: ({}, {}, {})\n", v0.position.x, v0.position.y, v0.position.z);
            //        print("    v1: ({}, {}, {})\n", v1.position.x, v1.position.y, v1.position.z);
            //        print("    v2: ({}, {}, {})\n", v2.position.x, v2.position.y, v2.position.z);
            //    }
            //}
        }
    }

    void ModelLoader::debugWriteEmbeddedTextures() const
    {
        const aiScene* scene = importer.GetScene();
        if (!scene || scene->mNumTextures == 0) {
            PRINT_TO_LOGGER("No embedded textures found in the model");
            return;
        }

        PRINT_TO_LOGGER("Found {} embedded textures, writing to debug files...", scene->mNumTextures);

        // Create debug directory if it doesn't exist
        string debugDir = "debug_textures";
        filesystem::create_directories(debugDir);

        for (uint32_t i = 0; i < scene->mNumTextures; ++i) {
            const aiTexture* aiTex = scene->mTextures[i];

            string filename;
            if (aiTex->mHeight == 0) {
                // Compressed texture data (PNG, JPG, etc.)
                // Try to determine format from the format hint
                string formatHint = string(aiTex->achFormatHint);
                if (formatHint.empty() || formatHint == "\0\0\0\0") {
                    // Try to detect format from data header
                    const unsigned char* data = reinterpret_cast<const unsigned char*>(aiTex->pcData);
                    if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
                        formatHint = "png";
                    }
                    else if (data[0] == 0xFF && data[1] == 0xD8) {
                        formatHint = "jpg";
                    }
                    else {
                        formatHint = "bin"; // Unknown format
                    }
                }

                filename = debugDir + "/embedded_texture_" + to_string(i) + "." + formatHint;

                // Write compressed data directly to file
                FILE* file = std::fopen(filename.c_str(), "wb");
                if (file) {
                    std::fwrite(aiTex->pcData, 1, aiTex->mWidth, file);
                    std::fclose(file);
                    PRINT_TO_LOGGER("Wrote compressed texture {}: {} ({} bytes)", i, filename, aiTex->mWidth);
                }
                else {
                    PRINT_TO_LOGGER("Failed to write compressed texture {}: {}", i, filename);
                }
            }
            else {
                // Uncompressed RGBA texture data
                filename = debugDir + "/embedded_texture_" + to_string(i) + ".png";

                // Convert aiTexel to RGBA8
                vector<unsigned char> rgba8Data(aiTex->mWidth * aiTex->mHeight * 4);
                for (uint32_t j = 0; j < static_cast<uint32_t>(aiTex->mWidth * aiTex->mHeight); ++j) {
                    rgba8Data[j * 4 + 0] = static_cast<unsigned char>(aiTex->pcData[j].r * 255);
                    rgba8Data[j * 4 + 1] = static_cast<unsigned char>(aiTex->pcData[j].g * 255);
                    rgba8Data[j * 4 + 2] = static_cast<unsigned char>(aiTex->pcData[j].b * 255);
                    rgba8Data[j * 4 + 3] = static_cast<unsigned char>(aiTex->pcData[j].a * 255);
                }

                // Write as PNG
                if (stbi_write_png(filename.c_str(), aiTex->mWidth, aiTex->mHeight, 4, rgba8Data.data(),
                    aiTex->mWidth * 4)) {
                    PRINT_TO_LOGGER("Wrote uncompressed texture {}: {} ({}x{})", i, filename, aiTex->mWidth,
                        aiTex->mHeight);
                }
                else {
                    PRINT_TO_LOGGER("Failed to write uncompressed texture {}: {}", i, filename);
                }
            }
        }

        PRINT_TO_LOGGER("Finished writing embedded textures to {} directory", debugDir);
    }

    void ModelLoader::optimizeMeshesBistro()
    {
        auto& meshes = model.meshes;
        auto& materials = model.materials;

        // 주의: mesh를 합친 후에는 그래프 구조에서도 합쳐진 것들을 반영시켜줘야 합니다.
        //      예: 그래프의 노드끼리도 합치기

        vector<string> materialNamesToMerge = { "Foliage_Linde_Tree_Large_Orange_Leaves",
                                               "Foliage_Linde_Tree_Large_Green_Leaves",
                                               "Foliage_Linde_Tree_Large_Trunk" };

        uint32_t totalMergedMeshes = 0;

        for (const auto& name : materialNamesToMerge) {

            vector<uint32_t> meshIndicesToMerge;
            for (uint32_t i = 0; i < meshes.size(); ++i) {
                if (materials[meshes[i].materialIndex].name == name &&
                    meshes[i].noTextureCoords == false) {
                    meshIndicesToMerge.push_back(i);
                }
            }

            if (meshIndicesToMerge.size() < 2) {
                PRINT_TO_LOGGER("No meshes found with material name '{}', skipping merge.", name);
                continue;
            }

            Mesh& firstMesh = meshes[meshIndicesToMerge[0]];
            // keep name of the first mesh

            // Merge vertices from all meshes into the first mesh
            uint32_t baseVertexCount = static_cast<uint32_t>(firstMesh.vertices.size());

            for (size_t i = 1; i < meshIndicesToMerge.size(); ++i) {
                uint32_t meshIndex = meshIndicesToMerge[i];
                Mesh& otherMesh = meshes[meshIndex];

                // Append vertices from other mesh
                firstMesh.vertices.insert(firstMesh.vertices.end(), otherMesh.vertices.begin(),
                    otherMesh.vertices.end());

                // Append indices from other mesh, adjusting them by the base vertex count
                uint32_t currentBaseVertexCount = baseVertexCount;
                for (uint32_t index : otherMesh.indices) {
                    firstMesh.indices.push_back(index + currentBaseVertexCount);
                }

                // Update base vertex count for next iteration
                baseVertexCount += static_cast<uint32_t>(otherMesh.vertices.size());
            }

            // Mark merged meshes for removal (except the first one)
            for (size_t i = 1; i < meshIndicesToMerge.size(); ++i) {
                uint32_t meshIndex = meshIndicesToMerge[i];
                meshes[meshIndex].vertices.clear();
                meshes[meshIndex].indices.clear();
            }

            totalMergedMeshes += static_cast<uint32_t>(meshIndicesToMerge.size() - 1);

            firstMesh.calculateBounds();

            PRINT_TO_LOGGER("Merged {} meshes with material '{}' into mesh {}", meshIndicesToMerge.size(),
                name, meshIndicesToMerge[0]);
        }

        // TODO: update following not to use copy assignment operator
        // Remove empty meshes (ones that were merged)
        auto writeIter = meshes.begin();
        for (auto readIter = meshes.begin(); readIter != meshes.end(); ++readIter) {
            if (!readIter->vertices.empty()) {
                if (writeIter != readIter) {
                    *writeIter = std::move(*readIter);
                }
                ++writeIter;
            }
        }
        meshes.erase(writeIter, meshes.end());

        PRINT_TO_LOGGER("Successfully optimized Bistro model");
        PRINT_TO_LOGGER("  Merged {} meshes", totalMergedMeshes);
        PRINT_TO_LOGGER("  Meshes after optimization: {}", meshes.size());
        PRINT_TO_LOGGER("  Materials: {}", materials.size());
    }

    void ModelLoader::processAnimations(const aiScene* scene)
    {
        if (!scene || scene->mNumAnimations == 0) {
            PRINT_TO_LOGGER("No animations found in the model");
            return;
        }

        PRINT_TO_LOGGER("Processing animations in VKModel...");
        PRINT_TO_LOGGER("  Scene has {} animations", scene->mNumAnimations);

        // Load animation data using our VKAnimation class
        // The VKAnimation system will calculate its own global inverse transform in loadFromScene
        model.animation->loadFromScene(scene);

        if (model.animation->hasAnimations()) {
            PRINT_TO_LOGGER("Successfully loaded {} animation clips", model.animation->getAnimationCount());
            PRINT_TO_LOGGER("  Current animation: '{}'", model.animation->getCurrentAnimationName());
            PRINT_TO_LOGGER("  Duration: {:.2f} seconds", model.animation->getDuration());
        }

        if (model.animation->hasBones()) {
            PRINT_TO_LOGGER("Successfully loaded {} bones for skeletal animation",
                model.animation->getBoneCount());
        }
    }

    void ModelLoader::processBones(const aiScene* scene)
    {
        if (!scene)
            return;

        // Check if any mesh has bones
        bool hasBones = false;
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
            if (scene->mMeshes[i]->HasBones()) {
                hasBones = true;
                break;
            }
        }

        if (!hasBones) {
            PRINT_TO_LOGGER("No bones found in any mesh");
            return;
        }

        uint32_t totalBones = 0;
        for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
            const aiMesh* mesh = scene->mMeshes[meshIndex];
            if (mesh->HasBones()) {
                totalBones += mesh->mNumBones;
            }
        }

        PRINT_TO_LOGGER("Total bones across all meshes: {}", totalBones);
    }

} // namespace vkengine