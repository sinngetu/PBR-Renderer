#version 410 core

out vec4 SV_Target;
in vec2 uv;

uniform sampler2D image;
uniform bool toneMapping = false;
uniform float exposure = 1.0;

vec3 ToneMapping(vec3 color) {
    return vec3(1.0) - exp(-color * exposure);
}

vec3 GammaCorrection(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec3 color = texture(image, uv).rgb;

    if (toneMapping) {
        color = ToneMapping(color);
    }

    color = GammaCorrection(color);
    SV_Target = vec4(color, 1.0);
}
