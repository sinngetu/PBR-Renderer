#version 410 core

in V2F {
    vec3 positionWS;
    vec4 positionLS;
    vec3 normalWS;
    vec2 uv;
} i;

out vec4 ST_Target;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

uniform sampler2D shadowMap;

uniform vec3 viewPosition;
uniform DirectionalLight light;

float getShadow(vec4 positionLS, float bias);
vec3 GammaCorrection(vec3 color);

void main() {
    vec3 N = normalize(i.normalWS);
    vec3 L = normalize(light.direction);
    vec3 V = normalize(viewPosition - i.positionWS);
    vec3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 256);
    float ambient = 0.1;

    vec3 color = light.color;
    float maxBias = 0.0008;
    float bias = max(maxBias * 0.1 * (1.0 - dot(N, L)), maxBias);
    float shadow = getShadow(i.positionLS, bias);

    color = ((1.0 - shadow) * (diffuse + specular) + ambient) * color;
    color = GammaCorrection(color);

    ST_Target = vec4(color, 1.0);
}

float getShadow(vec4 positionLS, float bias) {
    vec3 uv = positionLS.xyz / positionLS.w;

    // [-1,1] => [0,1]
    uv = (uv + 1.0) * 0.5;

    float currentDepth = uv.z;

    // Percentage-closer filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, uv.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }
    }

    shadow /= 9.0;

    if(currentDepth > 1.0)
        shadow = 0.0;

    return shadow;
}

vec3 GammaCorrection(vec3 color) {
    float gamma = 2.2;

    return pow(color, vec3(1.0 / gamma));
}
