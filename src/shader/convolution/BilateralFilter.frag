#version 410 core

out vec4 ST_Target;
in vec2 uv;

const float SIGMA_S = 5.0;
const float SIGMA_R = 40.0;
const int HALF_RANGE = 10;

uniform sampler2D image;

/*
 * Bilateral Filter
 *
 * Source:
 * Durand, F., Dorsey, J.: Fast bilateral filtering for the display of high-dynamic-range images. In: Proceedings of SIGGRAPH, pp.257–266. ACM, New York (2002)
 */
void main() {
    vec2 deltaUV = 1.0 / textureSize(image, 0);
    vec3 center = texture(image, uv).rgb;

    const float Gs_coef = -1.0 / (2.0 * SIGMA_S * SIGMA_S);
    const float Gr_coef = -1.0 / (2.0 * SIGMA_R * SIGMA_R);

    vec3 result = vec3(0.0);
    vec3 weight = vec3(0.0);

    for(int x = -HALF_RANGE; x < HALF_RANGE; x++) {
        for(int y = -HALF_RANGE; y < HALF_RANGE; y++) {
            vec3 value = texture(image, uv + vec2(deltaUV.x * x, deltaUV.y * y)).rgb;

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

    ST_Target = vec4(result, 1.0);
}
