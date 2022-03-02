#version 410 core

out vec4 SV_Target;
in vec2 uv;

uniform sampler2D image;
uniform float threshold = 1.0;

void main() {
    vec3 color = texture(image, uv).rgb;
    float L = dot(vec3(0.299, 0.587, 0.114), color);

    SV_Target = L > threshold ? vec4(color, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
