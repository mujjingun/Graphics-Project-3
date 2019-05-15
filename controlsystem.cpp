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

    if (scene.secondCamOn) {
        Camera& cam = scene.second;

        // rotate camera
        glm::vec3 right = glm::normalize(glm::cross(cam.upDir, cam.lookDir));

        if (input.isMouseDown()) {
            glm::vec2 angle = glm::vec2(input.mouseDelta()) * glm::radians(0.3f);
            float alt = glm::angle(cam.lookDir, cam.upDir);

            float min_angle = 5.0f;

            if (alt - angle.y < glm::radians(min_angle)) {
                angle.y = alt - glm::radians(min_angle);
            }
            if (alt - angle.y > glm::radians(180.0f - min_angle)) {
                angle.y = alt - glm::radians(180.0f - min_angle);
            }

            cam.lookDir = glm::rotate(glm::mat4(1.0), -angle.y, right)
                * glm::rotate(glm::mat4(1.0), angle.x, glm::vec3(cam.upDir))
                * glm::dvec4(cam.lookDir, 1.0);

            cam.lookDir = glm::normalize(cam.lookDir);
        }

        // move camera
        glm::vec3 moveDir{};
        glm::vec3 nLookDir = glm::normalize(cam.lookDir);
        glm::vec3 realUp = glm::cross(nLookDir, right);

        if (input.isKeyPressed('a')) {
            moveDir += right;
        }
        if (input.isKeyPressed('d')) {
            moveDir -= right;
        }
        if (input.isKeyPressed('s')) {
            moveDir -= nLookDir;
        }
        if (input.isKeyPressed('w')) {
            moveDir += nLookDir;
        }
        if (input.isKeyPressed('r')) {
            moveDir += realUp;
        }
        if (input.isKeyPressed('f')) {
            moveDir -= realUp;
        }

        if (glm::length(moveDir) > 0) {
            moveDir = glm::normalize(moveDir) * 1000.0f * deltaTime;
            cam.eyePos += moveDir;
        }

        cam.fov += input.scrollDelta();
        cam.fov = glm::clamp(cam.fov, 1.0f, 120.0f);
    } else {
        Camera& cam = scene.primary;

        if (input.isMouseDown() && input.isKeyPressed('+')) {
            cam.fov += input.mouseDelta().x * 0.4f;
            cam.fov = glm::clamp(cam.fov, 1.0f, 120.0f);
        }

        glm::vec3 right = glm::normalize(glm::cross(cam.upDir, cam.lookDir));

        float angle = glm::radians(11.25f);
        if (input.isKeyPressed('a')) {
            input.keyUp('a');
            cam.eyePos = cam.eyePos * glm::mat3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)));
        }
        if (input.isKeyPressed('d')) {
            input.keyUp('d');
            cam.eyePos = cam.eyePos * glm::mat3(glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0, 1, 0)));
        }
        if (input.isKeyPressed('w')) {
            input.keyUp('w');
            cam.eyePos = cam.eyePos * glm::mat3(glm::rotate(glm::mat4(1.0f), -angle, right));
        }
        if (input.isKeyPressed('s')) {
            input.keyUp('s');
            cam.eyePos = cam.eyePos * glm::mat3(glm::rotate(glm::mat4(1.0f), angle, right));
        }
        cam.lookDir = glm::normalize(-cam.eyePos);
    }

    // secondary camera
    if (input.isKeyPressed('v')) {
        input.keyUp('v');
        scene.secondCamOn = !scene.secondCamOn;
    }

    // car camera
    if (input.isKeyPressed('c')) {
        input.keyUp('c');
        scene.carViewportOn = !scene.carViewportOn;
    }

    // tiger camera
    if (input.isKeyPressed('t')) {
        input.keyUp('t');
        scene.tigerViewportOn = !scene.tigerViewportOn;
    }
}

void ControlSystem::afterUpdate(ou::ECSEngine& engine)
{
    Input& input = engine.getOne<Input>();
    input.afterUpdate();
}
