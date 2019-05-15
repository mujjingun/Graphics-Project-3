#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <deque>
#include <glm/glm.hpp>

struct Camera {
    glm::vec3 eyePos, lookDir, upDir;
    float fov = 45.0f;
};

struct SceneState {
    glm::ivec2 windowSize;
    Camera primary, second;
    bool carViewportOn = false;
    bool tigerViewportOn = false;
    bool secondCamOn = false;
};

struct Tiger {
    int currFrame = 0;
    float elapsedTime = 0;
    glm::vec3 pos{};
    glm::vec3 lastPos{};
};

struct Wolf {
    int currFrame = 0;
    float elapsedTime = 0;
};

struct Spider {
    int currFrame = 0;
    float elapsedTime = 0;
};

struct Teapot {
    glm::vec3 pos{};
    float angle = 0;
};

struct TigerCam {
};

struct Car {
    float elapsedTime = 1;
    float interval;
    float wheelAngle = 0;

    glm::vec2 pos{}, dir{};
    std::deque<glm::vec2> dests{};

    float angle{}, wheelRot{}, rearRot{};
};

struct CarCam {
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
