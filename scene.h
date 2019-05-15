#ifndef SCENE_H
#define SCENE_H

#include "ecs/ecsengine.h"
#include <chrono>

class Scene {
public:
    Scene();

    void render();

    void mouseClick(int button, int event);
    void mouseMove(int x, int y);
    void mouseEnter();
    void mouseLeft();

    void keyDown(unsigned char key);
    void keyUp(unsigned char key);

    void reshapeWindow(int width, int height);

private:
    ou::ECSEngine m_engine;
    std::chrono::system_clock::time_point m_lastFrame;
};

#endif // SCENE_H
