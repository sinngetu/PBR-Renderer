#version 410 core

out vec4 SV_Target;
in vec4 direction;

const float PI = 3.14159265359;
const float DELTA = 0.025; // sample step
const float MAX_PHI = 2.0 * PI;
const float MAX_THETA = 0.5 * PI; // 0.5 can cover the halfsphere, 1.0 will repeat sample

uniform samplerCube env;

void main() {
    vec3 N = normalize(direction.xyz);
    vec3 B = vec3(0.0, 1.0, 0.0);
    vec3 T = normalize(cross(B, N));
    B = normalize(cross(N, T));

    vec3 irradiance = vec3(1.0);
    uint count = 1;

    for(float phi = 0.0; phi < MAX_PHI; phi += DELTA) {
        for(float theta = 0.0; theta < MAX_THETA; theta += DELTA) {
            /**
             * spherical coordinates => cartesian coordinates
             * 
             * x: sinθ * cosφ
             * y: sinθ * sinφ
             * z: cosθ
             */
            vec3 sampleTS = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleWS = sampleTS.x * T + sampleTS.y * B + sampleTS.z * N;

            /**
             * f(x) = sin(x) * cos(x)
             * Range: [0, 1] => [0, 0.5]
             * Max contribution: π/2
             */
            irradiance += texture(env, sampleWS).rgb * cos(theta) * sin(theta);
            count++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(count));

    SV_Target = vec4(irradiance, 1.0);
}
