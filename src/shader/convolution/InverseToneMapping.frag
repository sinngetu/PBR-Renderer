#version 410 core

out vec4 SV_Target;
in vec4 direction;

const float N = 0.86; // generally between 0.7 (long test flashes) and 1.0 (short test flashes)

uniform samplerCube image;
uniform samplerCube sigmaMap;
uniform samplerCube surroundingMap;

uniform float maxValue; // max luminance of HDR

// RGB => Luminance, From: https://www.w3.org/TR/AERT/#color-contrast
float RGB2Luminance(vec3 color) {
    return dot(vec3(0.299, 0.587, 0.114), color);
}

/**
 * Inverse Tone Mapping(based on retina response)
 * 
 * Source:
 * Y. Huo, F. Yang, L. Dong, V. Brost. Physiological inverse tone mapping based on retina response, in: The Visual Computer, Vol. 30 No. 5, pp. 507-517, 2014.
 * 
 * Core Formula: R / Rmax = I^n / (I^n + σ^n)
 * 
 * [R] is the retina response
 * [Rmax] is the maximum response
 * [I] is the light intensity
 * [σ] is the global adaptation level, which represents the luminance required to generate a response one-half of the amplitude of Rmax
 * ----------------------------------------------------------------------
 * Usually, for LDR to HDR expansion,
 * only the luminance channel is processed, the chrominance channels are intact.
 * So, we operate the local function on the luminance channel:
 * 
 * Lh = (Ll / Lmax_l * (Ls_h^n + σ^n))^(1/n)
 */
void main() {
    vec3 color = texture(image, direction.xyz).rgb;
    float sigma_l = RGB2Luminance(texture(sigmaMap, direction.xyz).rgb);
    float Ls_l = RGB2Luminance(texture(surroundingMap, direction.xyz).rgb);

    float rate = maxValue / 255.0;

    float sigma = sigma_l * rate;
    float Ls_h = Ls_l * rate;

    float Lmax_l = sigma * 2.0; // max luminance of LDR

    float Ll = RGB2Luminance(color);
    float p1 = Ll / Lmax_l;
    float p2 = pow(Ls_h, N) + pow(sigma, N);

    float Lh = pow(p1*p2, 1/N);

    float scale = Lh / Ll;
    vec3 result = color * scale;

    SV_Target = vec4(result, 1.0);
}
