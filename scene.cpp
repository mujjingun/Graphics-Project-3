#include "scene.h"
#include "animationsystem.h"
#include "components.h"
#include "controlsystem.h"
#include "ecs/entity.h"
#include "rendersystem.h"
#include "input.h"

// clang-format off
#include <GL/glew.h>
#include <GL/freeglut.h>
// clang-format on

Scene::Scene()
    : m_engine{}
    , m_lastFrame{ std::chrono::system_clock::now() }
{
    SceneState state;
    state.eyePos = 4.0f / 6.0f * glm::vec3(500.0f, 600.0f, 500.0f);
    state.lookDir = glm::normalize(-state.eyePos);
    state.upDir = glm::vec3(0.0f, 1.0f, 0.0f);

    m_engine.addEntity(ou::Entity{ state, Input{} });
    m_engine.addEntity(ou::Entity{ Tiger{} });

    m_engine.addSystem(std::make_unique<AnimationSystem>());
    m_engine.addSystem(std::make_unique<ControlSystem>());
    m_engine.addSystem(std::make_unique<RenderSystem>(), 9);
}

void Scene::render()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    float delta = duration<float>(now - m_lastFrame).count();
    m_lastFrame = now;

    m_engine.update(delta);

    glutSwapBuffers();
}

void Scene::mouseClick(int button, int event)
{
    m_engine.getOne<Input>().mouseClick(button, event);
}

void Scene::mouseMove(int x, int y)
{
    m_engine.getOne<Input>().mouseMove(x, y);
}

void Scene::mouseEnter()
{
    m_engine.getOne<Input>().mouseEnter();
}

void Scene::keyDown(unsigned char key)
{
    m_engine.getOne<Input>().keyDown(key);
}

void Scene::keyUp(unsigned char key)
{
    m_engine.getOne<Input>().keyUp(key);
}

void Scene::reshapeWindow(int width, int height)
{
    glViewport(0, 0, width, height);
    m_engine.getOne<SceneState>().windowSize = glm::ivec2(width, height);
}
