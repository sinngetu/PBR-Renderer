#version 410 core

layout(location = 0) in vec3 positionOS;
layout(location = 1) in vec3 normalOS;
layout(location = 2) in vec2 texcoord;

out V2F {
    vec3 positionWS;
    vec4 positionLS;
    vec3 normalWS;
    vec2 uv;
} o;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 WorldToLight;

vec3 TransformObjectToWorldNormal(vec3 normalOS);

void main() {
    o.positionWS = (model * vec4(positionOS, 1.f)).xyz;
    o.positionLS = WorldToLight * vec4(o.positionWS, 1.f);
    o.normalWS = TransformObjectToWorldNormal(normalOS);
    o.uv = texcoord;

    gl_Position = projection * view * vec4(o.positionWS, 1.f);
}

vec3 TransformObjectToWorldNormal(vec3 normalOS) {
    return transpose(inverse(mat3(model))) * normalOS;
}
