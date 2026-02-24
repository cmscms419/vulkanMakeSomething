#ifndef INCLUDE_CONFIG_TYPE_H_
#define INCLUDE_CONFIG_TYPE_H_

#include "common.h"
#include "macros.h"

#include <vector>

// NEW: Model configuration structure
struct ModelConfig
{
    cString filePath;                       // Relative to assets path
    cString displayName;                    // Display name for GUI
    cMat4 transform = cMat4(1.0f); // Model transformation matrix
    cBool isBistroObj = false;              // Special handling for Bistro models

    // Animation settings
    cBool autoPlayAnimation = true;      // Start animation automatically
    cUint32_t initialAnimationIndex = 0; // Which animation to start with
    cFloat animationSpeed = 1.0f;        // Animation playback speed
    cBool loopAnimation = true;          // Loop the animation

    // Helper constructors
    ModelConfig() = default;

    ModelConfig(const cString& path, const cString& name = "",
        const cMat4& trans = cMat4(1.0f), cBool bistro = false)
        : filePath(path), displayName(name.empty() ? path : name), transform(trans),
        isBistroObj(bistro)
    {
    }

    // Fluent interface for easy configuration
    ModelConfig& setName(const cString& name)
    {
        displayName = name;
        return *this;
    }
    ModelConfig& setTransform(const cMat4& trans)
    {
        transform = trans;
        return *this;
    }
    ModelConfig& setBistroModel(cBool bistro)
    {
        isBistroObj = bistro;
        return *this;
    }
    ModelConfig& setAnimation(cBool autoPlay, cUint32_t index = 0, cFloat speed = 1.0f,
        cBool loop = true)
    {
        autoPlayAnimation = autoPlay;
        initialAnimationIndex = index;
        animationSpeed = speed;
        loopAnimation = loop;
        return *this;
    }
};

// NEW: Application configuration structure
struct ApplicationConfig
{
    std::vector<ModelConfig> models;

    static ApplicationConfig createDefault()
    {
        ApplicationConfig config;

        // Character model
        ModelConfig character("characters/Leonard/Bboy Hip Hop Move.fbx", "캐릭터");
        character.transform = glm::rotate(
            glm::scale(
            glm::translate(
            glm::mat4(1.0f), 
            glm::vec3(-6.719f, 0.375f, -1.860f)),
            glm::vec3(0.012f)),
            glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        character.autoPlayAnimation = true;

        // Bistro scene
        ModelConfig bistro("models/AmazonLumberyardBistroMorganMcGuire/exterior.obj", "거리",
            glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)),
            true // isBistroObj
        );
        bistro.autoPlayAnimation = false;

        config.models = { character, bistro };

        return config;
    }
};

#endif // !INCLUDE_CONFIG_TYPE_H_