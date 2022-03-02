#version 410 core

out vec4 SV_Target;
in vec2 uv;

uniform sampler2D image;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

// RGB => Luminance, From: https://www.w3.org/TR/AERT/#color-contrast
float RGB2Luminance(vec3 color) {
    return dot(vec3(0.299, 0.587, 0.114), color);
}

void main() {
    vec3 result = texture(image, uv).rgb * weight[0] * 2.0;
    vec2 texelSize = 1.0 / textureSize(image, 0);

    for(int i = 1; i < 5; ++i) {
        result += texture(image, uv + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        result += texture(image, uv - vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        result += texture(image, uv + vec2(0.0, texelSize.y * i)).rgb * weight[i];
        result += texture(image, uv - vec2(0.0, texelSize.y * i)).rgb * weight[i];
    }

    SV_Target = vec4(result * 0.5, 1.0);
}
