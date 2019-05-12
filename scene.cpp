#include "scene.h"
#include "components.h"
#include "ecs/entity.h"
#include "rendersystem.h"

// clang-format off
#include <GL/glew.h>
#include <GL/freeglut.h>
// clang-format on

Scene::Scene()
    : m_engine{}
    , m_lastFrame{ std::chrono::system_clock::now() }
{
    m_engine.addEntity(ou::Entity{ Tiger{} });

    m_engine.addSystem(std::make_unique<RenderSystem>());
}

void Scene::render()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    float delta = duration<float, std::milli>(now - m_lastFrame).count();
    m_lastFrame = now;

    m_engine.update(delta);

    glutSwapBuffers();
}

void Scene::mouseClick()
{
}

void Scene::mouseMove(int x, int y)
{
}

void Scene::mouseEnter()
{
}

void Scene::keyDown(unsigned char key)
{
}

void Scene::keyUp(unsigned char key)
{
}

void Scene::reshapeWindow(int width, int height)
{
}
