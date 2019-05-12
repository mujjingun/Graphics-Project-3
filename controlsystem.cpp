#include "controlsystem.h"
#include "components.h"
#include "ecs/ecsengine.h"
#include "ecs/entity.h"
#include "input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>

ControlSystem::ControlSystem()
{
}

void ControlSystem::update(ou::ECSEngine& engine, float deltaTime)
{
    // Update mouse cursor position
    SceneState& scene = engine.getOne<SceneState>();
    Input& input = engine.getOne<Input>();

    input.update(deltaTime, scene.windowSize);

    // rotate screen
    glm::dvec3 right = glm::cross(scene.upDir, scene.lookDir);

    glm::dvec2 angle = glm::dvec2(input.mouseDelta()) * glm::radians(0.3);
    float alt = glm::angle(scene.lookDir, scene.upDir);

    float min_angle = 5.0f;

    if (alt + angle.y < glm::radians(min_angle)) {
        angle.y = glm::radians(min_angle) - alt;
    }
    if (alt + angle.y > glm::radians(180.0f - min_angle)) {
        angle.y = glm::radians(180.0f - min_angle) - alt;
    }

    scene.lookDir = glm::rotate(glm::dmat4(1.0), angle.y, right)
        * glm::rotate(glm::dmat4(1.0), -angle.x, glm::dvec3(scene.upDir))
        * glm::dvec4(scene.lookDir, 1.0);

    scene.lookDir = glm::normalize(scene.lookDir);
}
