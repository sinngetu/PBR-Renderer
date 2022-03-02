#version 410 core

out vec4 SV_Target;
in vec2 uv;

const float SIGMA_S = 5.0;
const float SIGMA_R = 40.0;
const int HALF_RANGE = 10;

uniform sampler2D image;

uniform float compression;
uniform float detailScale;
uniform float scale;

// RGB => Luminance, From: https://www.w3.org/TR/AERT/#color-contrast
float RGB2Luminance(vec3 color) {
    return dot(vec3(0.299, 0.587, 0.114), color);
}

/*
 * Source:
 * Durand, F., Dorsey, J.: Fast bilateral filtering for the display of high-dynamic-range images. In: Proceedings of SIGGRAPH, pp.257–266. ACM, New York (2002)
 */
void main() {
    vec2 deltaUV = 1.0 / textureSize(image, 0);
    vec3 Cw = texture(image, uv).rgb;
    float Lw = log(RGB2Luminance(Cw));

    const float Gs_coef = -1.0 / (2.0 * SIGMA_S * SIGMA_S);
    const float Gr_coef = -1.0 / (2.0 * SIGMA_R * SIGMA_R);

    float base = 0.0;
    float weight = 0.0;

    for(int x = -HALF_RANGE; x < HALF_RANGE; x++) {
        for(int y = -HALF_RANGE; y < HALF_RANGE; y++) {
            vec2 sampleUV = uv + vec2(deltaUV.x * x, deltaUV.y * y);
            vec4 data = texture(image, sampleUV);
            float value = log(RGB2Luminance(data.rgb));

            if (sampleUV.x >= 0.0 && sampleUV.y >= 0.0 && sampleUV.x <= 1.0 && sampleUV.y <= 1.0 && data.a > 0.0) {
                float Gs_diff = x*x + y*y;
                float Gr_diff = 255.0 * (Lw - value); // zoom difference
                Gr_diff *= Gr_diff;

                float Gs = exp(Gs_diff * Gs_coef);
                float Gr = exp(Gr_diff * Gr_coef);

                float Wp = Gs * Gr;

                base += Wp * value;
                weight += Wp;
            }
        }
    }

    base /= weight;

    float detail = Lw - base;
    float Ld = (compression * base + detailScale * detail) * scale;
    Ld = exp(Ld);

    vec3 Cd = Ld/Lw * Cw;
    SV_Target = vec4(Cd, 1.0);
}
