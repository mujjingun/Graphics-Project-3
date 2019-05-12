#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <glm/glm.hpp>

struct SceneState {
    glm::ivec2 windowSize;
};

struct Tiger {
    int currFrame = 0;
    float rotationAngle = 0.0f;
};

struct Light {
    bool on = false;
    glm::vec4 pos;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec3 spotDir;
    float spotCutoffAngle;
    float spotExponent;
};

#endif // COMPONENTS_H
