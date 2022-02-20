#version 410 core

out vec4 ST_Target;
in vec4 direction;

const float PI = 3.14159265359;
const float DELTA = 0.025; // sample step
const float MAX_PHI = 2.0 * PI;
const float MAX_THETA = 0.5 * PI; // 0.5 can cover the halfsphere, 1.0 will repeat sample

uniform samplerCube env;

void main() {
    vec3 N = vec3(0.0, 1.0, 0.0);
    vec3 B = normalize(direction.xyz);
    vec3 T = normalize(cross(B, N));
    N = normalize(cross(B, T));

    vec3 irradiance = vec3(1.0);
    uint count = 1;

    for(float phi = 0.0; phi < MAX_PHI; phi += DELTA) {
        for(float theta = 0.0; theta < MAX_THETA; theta += DELTA) {
            /**
             * spherical coordinates => cartesian coordinates
             * 
             * x: sinθ * cosφ
             * y: θ
             * z: sinθ * cosφ
             */
            vec3 sampleTS = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
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

    ST_Target = vec4(irradiance, 1.0);
}
