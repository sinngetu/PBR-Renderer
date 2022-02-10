#version 410 core
layout (location = 0) in vec3 positionOS;

uniform mat4 WorldToLight;
uniform mat4 model;

void main() {
    vec3 positionWS = (model * vec4(positionOS, 1.0)).xyz;
    gl_Position = WorldToLight * vec4(positionWS, 1.0);
}
