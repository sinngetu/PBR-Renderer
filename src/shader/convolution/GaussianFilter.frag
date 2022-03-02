#version 410 core

out vec4 SV_Target;
in vec2 uv;

uniform sampler2D image;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// Gaussian Filter
void main() {
    vec3 result = texture(image, uv).rgb * weight[0] * 2.0; // two direction sampling: horizontal and vertical
    vec2 delta = 1.0 / textureSize(image, 0);

    for(int i = 1; i < 5; i++) {
        vec2 offset = i * delta;

        result += texture(image, uv + vec2(-offset.x, 0.0)).rgb * weight[i];
        result += texture(image, uv + vec2( offset.x, 0.0)).rgb * weight[i];
        result += texture(image, uv + vec2(0.0,  offset.y)).rgb * weight[i];
        result += texture(image, uv + vec2(0.0, -offset.y)).rgb * weight[i];
    }

    result /= 2.0;
    SV_Target = vec4(result, 1.0);
}
