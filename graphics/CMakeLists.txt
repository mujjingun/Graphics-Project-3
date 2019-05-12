
find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)
find_package(FreeImage REQUIRED)

add_library(graphics
    framebuffer.cpp
    rawbufferview.cpp
    renderbuffer.cpp
    shader.cpp
    texture.cpp
    vertexarray.cpp
    vertexbuffer.cpp
)

target_link_libraries(graphics
    ${FREEIMAGE_LIBRARIES}
    GLEW::GLEW
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${GLM_LIBRARIES}
)

target_include_directories(graphics PRIVATE ${CMAKE_SOURCE_DIR})
