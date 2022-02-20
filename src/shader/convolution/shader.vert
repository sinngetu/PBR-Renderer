#version 410 core
layout (location = 0) in vec3 positionOS;

void main() {
    gl_Position = vec4(positionOS, 1.0);
}
