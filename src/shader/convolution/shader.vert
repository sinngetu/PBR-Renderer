#version 410 core
layout (location = 0) in vec3 positionOS;
layout (location = 1) in vec2 texcoord;

out vec2 uv;

void main() {
    uv = texcoord;
    gl_Position = vec4(positionOS, 1.0);
}
