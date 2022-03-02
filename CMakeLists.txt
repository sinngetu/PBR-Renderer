cmake_minimum_required(VERSION 3.10)
project(Renderer)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

include_directories(${Renderer_SOURCE_DIR}/include)
link_libraries(${Renderer_SOURCE_DIR}/lib/libglad.dylib)
link_libraries(${Renderer_SOURCE_DIR}/lib/libglfw.3.dylib)
link_libraries(${Renderer_SOURCE_DIR}/lib/libassimp.5.dylib)

add_executable (Renderer
# entry
    src/main.cpp
    src/global.cpp

# utils
    src/util/debug.cpp
    src/util/convolution.cpp

# materials
    src/material/default.cpp
    src/material/PBR.cpp

# scenes
    src/scene/demo.cpp
)

target_link_libraries(Renderer ${OpenGL_LIBRARIES})
