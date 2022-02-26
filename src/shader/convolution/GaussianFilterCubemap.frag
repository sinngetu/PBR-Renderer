#version 410 core

out vec4 ST_Target;
in vec4 direction;

const float PI = 3.14159265359;
const int HALF_RANGE = 10;

uniform samplerCube image;
uniform float radius = 256.0;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// Gaussian Filter
void main() {
    vec3 result = texture(image, direction.xyz).rgb * weight[0] * 2.0; // two direction sampling: horizontal and vertical

    vec3 N = normalize(direction.xyz);
    vec3 B = vec3(0.0, 1.0, 0.0);
    vec3 T = normalize(cross(B, N));
    B = normalize(cross(N, T));

    float delta = 0.5 * PI / radius;

    // spherical local sampling
    for(int i = 1; i < 5; i++) {
        float offset = i * delta;

        vec3 upTS     = vec3(sin( offset) * cos(0.0), sin( offset) * sin(0.0), cos( offset));
        vec3 buttonTS = vec3(sin(-offset) * cos(0.0), sin(-offset) * sin(0.0), cos(-offset));
        vec3 leftTS   = vec3(sin(0.0) * cos(-offset), sin(0.0) * sin(-offset), cos(0.0));
        vec3 rightTS  = vec3(sin(0.0) * cos( offset), sin(0.0) * sin( offset), cos(0.0));

        vec3 upWS     = upTS.x     * T + upTS.y     * B + upTS.z     * N;
        vec3 buttonWS = buttonTS.x * T + buttonTS.y * B + buttonTS.z * N;
        vec3 leftWS   = leftTS.x   * T + leftTS.y   * B + leftTS.z   * N;
        vec3 rightWS  = rightTS.x  * T + rightTS.y  * B + rightTS.z  * N;

        result += texture(image, upWS    ).rgb * weight[i];
        result += texture(image, buttonWS).rgb * weight[i];
        result += texture(image, leftWS  ).rgb * weight[i];
        result += texture(image, rightWS ).rgb * weight[i];
    }

    result /= 2.0;
    ST_Target = vec4(result, 1.0);
}
