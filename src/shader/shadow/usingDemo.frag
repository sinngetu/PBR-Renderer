#version 410 core

in V2F {
    vec3 positionWS;
    vec4 positionLS;
    vec3 normalWS;
    vec2 uv;
} i;

out vec4 ST_Target;

uniform sampler2D shadowMap;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

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

void main() {
    // float maxBias = 0.005;  // try adjusting this parameter to match graphic
    // float bias = max(maxBias * 0.1 * (1.0 - dot(N, L)), maxBias);
    // float shadow = getShadow(i.positionLS, bias);
    // vec3 lighting = ((1.0 - shadow) * (diffuse + specular) + ambient) * color;

    // ST_Target = vec4(lighting, 1.0);
}
