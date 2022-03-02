#version 410 core

out vec4 SV_Target;
in vec2 uv;

uniform sampler2D image;
uniform sampler2D bloom;

void main() {
    vec3 color = texture(image, uv).rgb;

    color += texture(bloom, uv).rgb;
    SV_Target = vec4(color, 1.0);
}
