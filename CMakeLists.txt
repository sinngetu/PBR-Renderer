cmake_minimum_required(VERSION 3.10)
project(Renderer)

find_package(OpenGL REQUIRED)

include_directories(${Renderer_SOURCE_DIR}/include)
link_libraries(${Renderer_SOURCE_DIR}/lib/libglad.dylib)
link_libraries(${Renderer_SOURCE_DIR}/lib/libglfw.3.dylib)
link_libraries(${Renderer_SOURCE_DIR}/lib/libassimp.5.dylib)

add_executable (Renderer src/main.cpp src/global.cpp)
target_link_libraries(Renderer ${OpenGL_LIBRARIES})
