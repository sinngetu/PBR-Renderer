#version 410 core

layout(location = 0) in vec3 positionOS;
layout(location = 1) in vec3 normalOS;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 tangentOS;
layout(location = 4) in vec3 bitangentOS;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

out V2F {
    vec3 positionTS;
    vec4 positionLS;
    vec3 viewPositionTS;
    DirectionalLight light;
    vec2 uv;
} o;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform DirectionalLight light;
uniform vec3 viewPosition;
uniform mat4 WorldToLight;

void main() {
    vec3 T = normalize(mat3(model) * tangentOS);
    vec3 B = normalize(mat3(model) * bitangentOS);
    vec3 N = normalize(mat3(model) * normalOS);
    mat3 TBN = transpose(mat3(T, B, N));

    vec3 positionWS = (model * vec4(positionOS, 1.0)).xyz;

    o.positionTS  = TBN * positionWS;
    o.positionLS = WorldToLight * vec4(positionWS, 1.0);
    o.viewPositionTS = TBN * viewPosition;
    o.light.direction = TBN * light.direction;
    o.light.color = light.color;
    o.uv = texcoord;

    gl_Position = projection * view * vec4(positionWS, 1.0);
}
