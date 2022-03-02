#version 410 core

struct Material {
    sampler2D baseColorMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D normalMap;
    sampler2D heightMap;
    sampler2D aoMap;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

in V2F {
    vec3 positionTS;
    vec4 positionLS;
    vec3 viewPositionTS;
    DirectionalLight light;
    vec2 uv;
} i;

out vec4 SV_Target;

uniform Material material;

// Indirect Light
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

// Shadow
uniform sampler2D shadowMap;

// Height Scale
uniform float heightScale;

const float PI = 3.14159265359;
const float MAX_PREFILTER_LOD = 4.0; // number of prefilterMap LOD

vec2  HeightMapping(vec2 uv, vec3 V);
float GGX(float NdotH, float roughness);
float GeometrySmith(float NdotV, float NdotL, float roughness);
float SchlickGGX(float NdotV, float roughness);
vec3  SchlickFresnel(float cosTheta, vec3 F0);
vec3  SchlickFresnelRoughness(float cosTheta, vec3 F0, float roughness);
float getShadow(vec4 positionLS, float bias);
vec3  ToneMapping(vec3 color);
vec3  GammaCorrection(vec3 color);

/*
 * Metallic/Roughness workflow
 * Computing Space: Tangent Space
 * Light Support: only one directional light
 */
void main() {
    vec3 V  = normalize(i.viewPositionTS - i.positionTS);
    vec2 uv = HeightMapping(i.uv, V);

    if (uv.x > 1.0 || uv.x < 0.0 || uv.y > 1.0 || uv.x < 0.0)
        discard;

    vec4  fragument = texture(material.baseColorMap, uv);
    vec3  baseColor = fragument.rgb;
    float metallic  = texture(material.metallicMap, uv).r;
    float roughness = texture(material.roughnessMap, uv).r;
    float ao        = texture(material.aoMap, uv).r;
    vec3  normal    = texture(material.normalMap, uv).xyz;
    float alpha = fragument.a;

    vec3 L  = normalize(i.light.direction);
    vec3 H  = normalize(L + V);
    vec3 N  = normalize(normal * 2.f - 1.f); // [0, 1] => [-1, 1]

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float attenuation = 1.0; // directional light have no attenuation by default
    vec3  radiance    = i.light.color * attenuation;

    vec3 F0 = vec3(0.04); // 4% basically covers common non-conductive materials
    F0 = mix(F0, baseColor, metallic); // use the base color as F0 of metal

    /**
     * direct light part
     * 
     * Cook-Torrance BRDF
     * f(l, v) = diffuse + D(θh)·F(θd)·G(θl,θv) / 4·cos(θl)·cos(θv)
     */
    float D = GGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    vec3  F = SchlickFresnel(HdotV, F0);

    // [NdotV & NdotL] reduction in [G & denominator]
    vec3 specular = D * G * F / 4.0;

    /**
     * diffuse BRDF deduction
     * 
     * area of sphere: θ is yaw angle, φ is pitch angle
     * A = (r·dθ)(r·sinθ·dφ)
     *   = r^2·sinθ·dθ·dφ
     * 
     * solid angle: (area of sphere) / (radius squared)
     * w = A / r^2
     *   = sinθ·dθ·dφ
     * 
     * halfsphere integral, assuming Li & Lo equality everywhere, so we can take Li as a constant
     * Lo = ∫(0, 2π) f_r·Li·cosθ dw
     *    = f_r * Li * ∫(0, 2π) cosθ dw
     *    = f_r * Li * ∫(0, π/2) cosθ·sinθ dθ * ∫(0, 2π) dφ
     *    = f_r * Li * ∫(0, 1) sinθ d(sinθ) * 2π
     *    = f_r * Li * ∫(0, 1) x d(x) * 2π
     *    = f_r * Li * π
     * 
     * ∵ Lo = Li (conservation of energy)
     * ∴ f_r = 1 / π
     */
    vec3 kD = vec3(1.0) - F; // distribute light to diffuse
    kD *= (1.0 - metallic);  // metallic part absorbs light

    vec3 diffuse = kD * baseColor / PI;

    const float MAX_BIAS = 0.0008; // may need to try adjustment
    float bias = max(MAX_BIAS * 0.1 * (1.0 - dot(N, L)), MAX_BIAS);
    float shadow = getShadow(i.positionLS, bias);

    vec3 direct = (diffuse + specular) * radiance * NdotL * (1.0 - shadow);

    /**
     * indirect light part
     * 
     * using IBL to calculate
     */
    F = SchlickFresnelRoughness(NdotV, F0, roughness);

    vec3 irradiance = texture(irradianceMap, N).rgb;

    kD = vec3(1.0) - F;     // distribute light to diffuse
    kD *= (1.0 - metallic); // metallic part absorbs light

    diffuse = kD * irradiance * baseColor;

    /**
     * F * coefficient + offset
     * 
     * Complete Formula:
     * F * ∫ f(p, ωi, ωo)(1 − (1 − ωo⋅h)^5)n⋅ωi dωi + ∫ f(p, ωi, ωo)((1 − ωo⋅h)^5)n⋅ωi dωi
     */
    vec3 prefilteredColor = textureLod(prefilterMap, reflect(-V, N), roughness * MAX_PREFILTER_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;

    specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 indirect = (diffuse + specular) * ao;

    vec3 color = direct + indirect;

    color = ToneMapping(color);
    // color = GammaCorrection(color);

    SV_Target = vec4(color, alpha);
}

vec2 HeightMapping(vec2 uv, vec3 V) {
    const float minLayers = 8;
    const float maxLayers = 32;

    // as the angle decreases(between view and plane), add the number of sample
    float numLayers  = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V)));
    float layerDepth = 1.f / numLayers; // sample step

    vec2 P = V.xy / V.z * heightScale; // with increase of z(means the angle becomes smaller), amplify offset value
    vec2 deltaUV = P / numLayers;

    vec2 nowUV = uv;
    float nowDepth = texture(material.heightMap, nowUV).r;
    float nowLayerDepth = 0.f;

    // find the layer previous to intersection 
    while(nowDepth > nowLayerDepth) {
        nowUV -= deltaUV;
        nowDepth = texture(material.heightMap, nowUV).r;
        nowLayerDepth += layerDepth;
    }

    vec2 prevUV = nowUV + deltaUV;

    // get layers depth
    float beforeDepth = texture(material.heightMap, prevUV).r - (nowLayerDepth - layerDepth);
    float afterDepth  = nowDepth - nowLayerDepth;

    float weight = afterDepth / (beforeDepth - afterDepth);

    // interpolation by weight
    return nowUV * weight + prevUV * (1.0 - weight);
}

// D = GGXTR(n, h, α) = α^2 / π((n·h)^2 * (α^2 - 1) + 1)^2
float GGX(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

// Smith Joint Masking-Shadowing Function
float GeometrySmith(float NdotV, float NdotL, float roughness) {
    float ggx2 = SchlickGGX(NdotV, roughness); // masking
    float ggx1 = SchlickGGX(NdotL, roughness); // shadowing

    return ggx1 * ggx2;
}

/*
 * G = SchlickGGX(n, v, k) = n·v / ((n·v)(1-k) + k)
 * 
 * direct: k = (α+1)^2 / 8
 * IBL: k = α^2 / 2
 */
float SchlickGGX(float NdotV, float roughness) {
    float a = roughness + 1.0;
    float k = (a * a) / 8.0;

    // NdotV reduction in [G & denominator]
    return NdotV / (NdotV * (1.0 - k) + k);
}

// F = SchlickFresnel(h, v) = F0 + (1 - F0)(1 - cosθ)^5
vec3 SchlickFresnel(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 SchlickFresnelRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

vec3 ToneMapping(vec3 color) {
    // return color / (color + 1.0);
    return vec3(1.0) - exp(-color);
}

vec3 GammaCorrection(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}
