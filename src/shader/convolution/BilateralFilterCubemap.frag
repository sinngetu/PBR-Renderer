#version 410 core

out vec4 SV_Target;
in vec4 direction;

const float PI = 3.14159265359;
const float SIGMA_S = 5.0;
const float SIGMA_R = 100.0;
const int HALF_RANGE = 10;

uniform samplerCube image;

/*
 * Bilateral Filter
 *
 * Source:
 * Durand, F., Dorsey, J.: Fast bilateral filtering for the display of high-dynamic-range images. In: Proceedings of SIGGRAPH, pp.257–266. ACM, New York (2002)
 */
void main() {
    vec3 center = texture(image, direction.xyz).rgb;

    const float Gs_coef = -1.0 / (2.0 * SIGMA_S * SIGMA_S);
    const float Gr_coef = -1.0 / (2.0 * SIGMA_R * SIGMA_R);

    vec3 result = vec3(0.0);
    vec3 weight = vec3(0.0);

    vec3 N = normalize(direction.xyz);
    vec3 B = vec3(0.0, 1.0, 0.0);
    vec3 T = normalize(cross(B, N));
    B = normalize(cross(N, T));

    float delta = 0.5 * PI / 512.0; // face range = 2π / 4, 512 is a empirical parameters

    // spherical local sampling
    for(int x = -HALF_RANGE; x < HALF_RANGE; x++) {
        for(int y = -HALF_RANGE; y < HALF_RANGE; y++) {
            float phi   = x * delta;
            float theta = y * delta;

            vec3 sampleTS = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleWS = sampleTS.x * T + sampleTS.y * B + sampleTS.z * N;

            vec3 value = texture(image, sampleWS).rgb;

            float Gs_diff = x*x + y*y;
            vec3  Gr_diff = 255.0 * (center - value); // zoom difference
            Gr_diff *= Gr_diff;

            float Gs = exp(Gs_diff * Gs_coef);
            vec3  Gr = exp(Gr_diff * Gr_coef);

            vec3 Wp = Gs * Gr;

            result += Wp * value;
            weight += Wp;
        }
    }

    result /= weight;

    SV_Target = vec4(result, 1.0);
}
