#include <iostream>

// clang-format off
#include <GL/glew.h>
#include <GL/freeglut.h>
// clang-format on

#include "scene.h"

static Scene* pScene;

static void renderScene()
{
    pScene->render();
}

static void timer(int)
{
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

static void keyboardDown(unsigned char key, int, int)
{
    pScene->keyDown(key);
}

static void keyboardUp(unsigned char key, int, int)
{
    pScene->keyUp(key);
}

static void specialKeyboardDown(int key, int, int)
{
    switch (key) {
    case GLUT_KEY_LEFT:
        pScene->keyDown('a');
        break;
    case GLUT_KEY_RIGHT:
        pScene->keyDown('d');
        break;
    case GLUT_KEY_UP:
        pScene->keyDown('w');
        break;
    case GLUT_KEY_DOWN:
        pScene->keyDown('s');
        break;
    }
}

static void specialKeyboardUp(int key, int, int)
{
    switch (key) {
    case GLUT_KEY_LEFT:
        pScene->keyUp('a');
        break;
    case GLUT_KEY_RIGHT:
        pScene->keyUp('d');
        break;
    case GLUT_KEY_UP:
        pScene->keyUp('w');
        break;
    case GLUT_KEY_DOWN:
        pScene->keyUp('s');
        break;
    }
}

static void mouseMove(int x, int y)
{
    pScene->mouseMove(x, y);
}

static void mouseEvent(int button, int state, int, int)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        pScene->mouseClick();
    }
}

static void mouseEntry(int state)
{
    if (state == GLUT_ENTERED) {
        pScene->mouseEnter();
    }
}

static void reshapeWindow(int width, int height)
{
    pScene->reshapeWindow(width, height);
}

static void GLAPIENTRY openglDebugCallback(GLenum source, GLenum type, GLenum id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam)
{
    const char* type_str;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "MARKER";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        type_str = "PUSH_GROUP";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        type_str = "POP_GROUP";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "OTHER";
        break;
    }

    const char* sev_str;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        sev_str = "HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        sev_str = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        sev_str = "LOW";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        sev_str = "NOTIFICATION";
    }

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        std::cout << "OpenGL Message: " << message
                  << " type=" << type_str
                  << " severity=" << sev_str
                  << "\n";

    } else {
        std::cerr << "OpenGL Message: " << message
                  << " type=" << type_str
                  << " severity=" << sev_str
                  << "\n";
    }
}

static void initialize_glew()
{
    GLenum error;

    glewExperimental = GL_TRUE;

    error = glewInit();
    if (error != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
        exit(-1);
    }
    fprintf(stdout, "*********************************************************\n");
    fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
    fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
    fprintf(stdout, "*********************************************************\n\n");
}

int main(int argc, char* argv[])
{
    // initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 800);
    glutInitContextVersion(4, 5);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Sogang CSE4170 3D Objects");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    // initialize glew
    initialize_glew();

    // register callbacks
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyboardDown);
    glutSpecialUpFunc(specialKeyboardUp);
    glutPassiveMotionFunc(mouseMove);
    glutEntryFunc(mouseEntry);
    glutReshapeFunc(reshapeWindow);
    glutMouseFunc(mouseEvent);
    glutDisplayFunc(renderScene);
    glutTimerFunc(0, timer, 0);

    // print opengl debug output
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglDebugCallback, nullptr);

    try {
        Scene scene;
        pScene = &scene;
        glutMainLoop();
    } catch (std::exception& e) {
        std::cerr << "Exception thrown: " << e.what() << std::endl;
    }

    glutExit();

    std::cout << "Exiting..." << std::endl;

    return 0;
}
