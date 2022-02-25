#version 410 core

in vec2 uv;
out vec4 ST_Target;

const float PI = 3.14159265359;

vec2 IntegrateBRDF(float NdotV, float roughness);
float radicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 random, vec3 N, float roughness);
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness);
float SchlickGGX(float NdotV, float roughness);

void main() {
    vec2 BRDF = IntegrateBRDF(uv.x, uv.y);
    ST_Target = vec4(BRDF, 0.0, 1.0);
}

/**
 * From: Epic Games\UE_4.20\Engine\Shaders\Private\MonteCarlo.ush
 * 
 * float2 IntegrateBRDF(float Roughness, float NoV)
 *     {
 *         float3 V;
 *         V.x = sqrt(1.0f - NoV * NoV); // sin
 *         V.y = 0;
 *         V.z = NoV; // cos
 *         float A = 0;
 *         float B = 0;
 *         const uint NumSamples = 1024;
 *         for (uint i = 0; i < NumSamples; i++ )
 *         {
 *             float2 Xi = Hammersley(i, NumSamples);
 *             float3 H = ImportanceSampleGGX(Xi, Roughness, N);
 *             float3 L = 2 * dot(V, H) * H - V;
 *             float NoL = saturate(L.z);
 *             float NoH = saturate(H.z);
 *             float VoH = saturate(dot(V, H));
 *             if (NoL > 0) {
 *                 float G = G_Smith(Roughness, NoV, NoL);
 *                 float G_Vis = G * VoH / (NoH * NoV);
 *                 float Fc = pow(1 - VoH, 5);
 *                 A += (1 - Fc) * G_Vis;
 *                 B += Fc * G_Vis;
 *             }
 *         }
 *         return float2(A, B) / NumSamples;
 *     }
 */
vec2 IntegrateBRDF(float NdotV, float roughness) {
    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);
    const uint NUM_SAMPLES = 1024u;
    for(uint i = 0u; i < NUM_SAMPLES; i++) {
        vec2 Xi = Hammersley(i, NUM_SAMPLES);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness); // generate sample vector(importance sampling)
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        /**
         * F * coefficient + offset
         * 
         * Complete Formula:
         * F * ∫ f(p, ωi, ωo)(1 − (1 − ωo⋅h)^5)n⋅ωi dωi + ∫ f(p, ωi, ωo)((1 − ωo⋅h)^5)n⋅ωi dωi
         */
        if(NdotL > 0.0) {
            float G = G_Smith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0); // (1 − ωo⋅h)^5

            A += (1.0 - Fc) * G_Vis; // coefficient
            B += Fc * G_Vis;         // offset
        }
    }

    A /= float(NUM_SAMPLES);
    B /= float(NUM_SAMPLES);
    return vec2(A, B);
}

// From: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html#sec-SourceCode
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // 0x100000000
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

/**
 * From: Epic Games/UE_4.20/Engine/Shaders/Private/MonteCarlo.ush
 * 
 * float4 ImportanceSampleGGX( float2 E, float a2 )
 * {
 *      float Phi = 2 * PI * E.x;
 *      float CosTheta = sqrt( (1 - E.y) / ( 1 + (a2 - 1) * E.y ) );
 *      float SinTheta = sqrt( 1 - CosTheta * CosTheta );
 * 
 * 	    float3 H;
 * 	    H.x = SinTheta * cos( Phi );
 * 	    H.y = SinTheta * sin( Phi );
 * 	    H.z = CosTheta;
 * 
 * 	    float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
 * 	    float D = a2 / ( PI*d*d );
 * 	    float PDF = D * CosTheta;
 * 
 * 	    return float4( H, PDF );
 * }
 */

vec3 ImportanceSampleGGX(vec2 random, vec3 N, float roughness) {
	float a = roughness * roughness;
    float a2 = a * a;

	float phi = 2.0 * PI * random.x;

	float cosTheta = sqrt((1.0 - random.y) / (1.0 + (a2 - 1.0) * random.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// spherical coordinates => cartesian coordinates
	vec3 H;
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;

	// tangent space => world space
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

// Smith Joint Masking-Shadowing Function
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = SchlickGGX(NdotV, roughness); // masking
    float ggx1 = SchlickGGX(NdotL, roughness); // shadowing

    return ggx1 * ggx2;
}

float SchlickGGX(float NdotV, float roughness) {
    float a = roughness;
    float k = (a * a) / 2.0; // k = a^2 / 2

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
