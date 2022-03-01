#version 410 core

in vec2 uv;

out vec4 SV_Target;

uniform bool singleColor;
uniform sampler2D theTexture;

void main() {
    vec3 color = singleColor ? vec3(texture(theTexture, uv).r) : texture(theTexture, uv).rgb;

    SV_Target = vec4(color, 1.0);
}
