#include "input.h"

// clang-format off
#include <GL/glew.h>
#include <GL/freeglut.h>
// clang-format on

#include <iostream>

void Input::keyDown(unsigned char key)
{
    m_keyStates[key] = true;
}

void Input::keyUp(unsigned char key)
{
    m_keyStates[key] = false;
}

void Input::mouseClick(int button, int event)
{
    if (button == GLUT_LEFT_BUTTON) {
        m_leftDown = (event == GLUT_DOWN);

        if (event == GLUT_UP) {
            m_mouseClicked = true;
        }

    } else if (button == GLUT_RIGHT_BUTTON) {
        m_rightDown = (event == GLUT_DOWN);
    }

    if (m_doCaptureMouse) {
        if (button == GLUT_LEFT_BUTTON && event == GLUT_DOWN) {
            if (m_mouseCaptured) {
                glutSetCursor(GLUT_CURSOR_INHERIT);
                m_mouseCaptured = false;
            } else {
                glutSetCursor(GLUT_CURSOR_NONE);
                m_mouseCaptured = true;
            }
        }
    }
}

void Input::mouseMove(int x, int y)
{
    m_realMousePos = { x, y };

    auto diff = glm::abs(m_realMousePos - m_lastRealMousePos);
    if (m_mouseInvalidated && (diff.x > 50 || diff.y > 50)) {
        m_mouseInvalidated = false;
        m_lastRealMousePos = m_realMousePos;
    }
}

void Input::mouseEnter()
{
    m_mouseInvalidated = true;
    m_mouseInScreen = true;
}

void Input::mouseLeft()
{
    m_mouseInScreen = false;
}

void Input::mouseScroll(int delta)
{
    m_mouseWheelDelta += delta;
}

void Input::update(float delta, glm::ivec2 windowSize)
{
    m_destLogicalMousePos += m_realMousePos - m_lastRealMousePos;
    m_lastRealMousePos = m_realMousePos;

    // apply smoothing
    double smoothing = 1 - glm::exp(-double(delta) * 15.0);
    m_smoothedMouseDelta = (m_destLogicalMousePos - m_logicalMousePos) * smoothing;
    m_logicalMousePos += m_smoothedMouseDelta;

    // capture mouse
    if (!m_mouseInvalidated && m_mouseCaptured) {
        if (m_realMousePos.x > windowSize.x * .6
            || m_realMousePos.x < windowSize.x * .4
            || m_realMousePos.y > windowSize.y * .6
            || m_realMousePos.y < windowSize.y * .4) {
            glutWarpPointer(windowSize.x / 2, windowSize.y / 2);
            m_mouseInvalidated = true;
        }
    }
}

void Input::afterUpdate()
{
    m_mouseWheelDelta = 0;
    m_mouseClicked = false;
}

bool Input::isMouseDown() const
{
    return m_leftDown;
}

bool Input::isKeyPressed(unsigned char key) const
{
    auto it = m_keyStates.find(key);
    if (it != m_keyStates.end()) {
        return it->second;
    }
    return false;
}

bool Input::isMouseClicked() const
{
    return m_mouseClicked;
}

bool Input::isMouseInScreen() const
{
    return m_mouseInScreen;
}

glm::ivec2 Input::mousePos() const
{
    return m_realMousePos;
}

glm::dvec2 Input::mouseDelta() const
{
    return m_smoothedMouseDelta;
}

int Input::scrollDelta() const
{
    return m_mouseWheelDelta;
}
