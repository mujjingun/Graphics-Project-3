cmake_minimum_required(VERSION 3.10)

project(graphics3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)
find_package(FreeImage REQUIRED)

add_subdirectory(graphics)
add_subdirectory(ecs)

add_executable(graphics3
    #Tiger_Texture_PS_GLSL.cpp
    #Shaders/LoadShaders.cpp
    main.cpp
    scene.cpp
    rendersystem.cpp
    animationsystem.cpp
)

target_link_libraries(graphics3
    graphics
    ecs
    ${GLUT_LIBRARIES}
)
