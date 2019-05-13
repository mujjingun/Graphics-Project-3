#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <deque>
#include <glm/glm.hpp>

struct SceneState {
    glm::ivec2 windowSize;
    glm::vec3 eyePos, lookDir, upDir;
};

struct Tiger {
    int currFrame = 0;
    float elapsedTime = 0;
    glm::vec3 pos{};
    glm::vec3 lastPos{};
};

struct Car {
    float elapsedTime = 1;
    float interval;
    float wheelAngle = 0;

    glm::vec2 pos{};
    std::deque<glm::vec2> dests{};

    float angle{}, wheelRot{};
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
