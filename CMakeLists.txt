
cmake_minimum_required(VERSION 3.12)

project(BlueMarble)

add_executable(BlueMarble main.cpp
                          Camera.cpp)

target_include_directories(BlueMarble PRIVATE deps/glm 
                                              deps/glfw/include
                                              deps/glew/include
                                              deps/stb)

target_link_directories(BlueMarble PRIVATE deps/glfw/lib-vc2019
                                           deps/glew/lib/Release/x64)

target_link_libraries(BlueMarble PRIVATE glfw3.lib glew32.lib opengl32.lib)

add_custom_command(TARGET BlueMarble POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/glew/bin/Release/x64/glew32.dll" "${CMAKE_BINARY_DIR}/glew32.dll"
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/shaders/triangle_vert.glsl" "${CMAKE_BINARY_DIR}/shaders/triangle_vert.glsl"
				   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/shaders/triangle_frag.glsl" "${CMAKE_BINARY_DIR}/shaders/triangle_frag.glsl"
				   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/textures/earth_2k.jpg" "${CMAKE_BINARY_DIR}/textures/earth_2k.jpg"
				   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/textures/earth_clouds_2k.jpg" "${CMAKE_BINARY_DIR}/textures/earth_clouds_2k.jpg"
                   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/textures/earth5400x2700.jpg" "${CMAKE_BINARY_DIR}/textures/earth5400x2700.jpg")

add_executable(Vetores Vectors.cpp)
target_include_directories(Vetores PRIVATE deps/glm)

add_executable(Matrizes Matrices.cpp)
target_include_directories(Matrizes PRIVATE deps/glm)