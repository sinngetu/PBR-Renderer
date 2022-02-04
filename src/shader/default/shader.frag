#version 410 core

in V2F {
    vec3 positionWS;
    vec3 normalWS;
    vec2 uv;
} i;

out vec4 ST_Target;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

uniform vec3 viewPosition;
uniform DirectionalLight light;

vec3 GammaCorrection(vec3 color);

void main() {
    vec3 N = normalize(i.normalWS);
    vec3 L = normalize(-light.direction);
    vec3 V = normalize(viewPosition - i.positionWS);
    vec3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 256);
    float ambient = 0.1;

    vec3 color = light.color;
    color = (diffuse + specular + ambient) * color;
    color = GammaCorrection(color);

    ST_Target = vec4(color, 1.0);
}

vec3 GammaCorrection(vec3 color) {
    float gamma = 2.2;

    return pow(color, vec3(1.0 / gamma));
}
