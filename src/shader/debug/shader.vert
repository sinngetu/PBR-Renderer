#version 410 core

layout (location = 0) in vec2 positionOS;
layout (location = 1) in vec2 texCoords;

out vec2 uv;

void main() {
    gl_Position = vec4(positionOS, 0.0, 1.0); 
    uv = texCoords;
}
