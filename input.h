#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include <unordered_map>

class Input {

public:
    Input() = default;

    void keyDown(unsigned char key);
    void keyUp(unsigned char key);

    void mouseClick(int button, int event);
    void mouseMove(int x, int y);
    void mouseEnter();
    void mouseLeft();
    void mouseScroll(int delta);

    void update(float delta, glm::ivec2 windowSize);
    void afterUpdate();

    bool isMouseDown() const;
    bool isKeyPressed(unsigned char key) const;

    glm::dvec2 mouseDelta() const;
    int scrollDelta() const;

private:
    // keyboard
    std::unordered_map<unsigned char, bool> m_keyStates;

    // mouse
    glm::ivec2 m_lastRealMousePos{};
    glm::ivec2 m_realMousePos{};
    glm::dvec2 m_logicalMousePos{};
    glm::dvec2 m_destLogicalMousePos{};
    glm::dvec2 m_smoothedMouseDelta{};

    bool m_mouseInvalidated = false;
    bool m_leftDown = false;
    bool m_rightDown = false;
    bool m_doCaptureMouse = false;
    bool m_mouseCaptured = false;

    int m_mouseWheelDelta = 0;
};

#endif // INPUT_H
