#version 410 core

out vec4 SV_Target;
in vec4 direction;

uniform samplerCube env;
uniform float roughness;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 4096u;

float GGX(vec3 N, vec3 H, float roughness);
float radicalInverse_VdC(uint bits);
vec2  Hammersley(uint i, uint N);
vec3  ImportanceSampleGGX(vec2 random, vec3 N, float roughness);

void main() {
    vec3 N = normalize(direction.xyz); // direction of out radiance
    vec3 V = N; // make view direction always equal out direction, discard the out direction precision

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    for(uint i = 0u; i < SAMPLE_COUNT; i++) {
        vec2 random = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(random, N, roughness); // generate sample vector(importance sampling)
        vec3 L  = normalize(2.0 * dot(V, H) * H - V); // L+V = 2 * H(V·H)

        float NdotL = max(dot(N, L), 0.0);

        if(NdotL > 0.0) {
            // reduce artifact, sample from the environment's mip level based on roughness/pdf
            float D = GGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; // +0.0001 avoid 0, same below

            float resolution = 512.0; // resolution of source cubemap (per face)
            float texel  = 4.0 * PI / (6.0 * resolution * resolution);
            float sampled = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(sampled / texel); 

            color += textureLod(env, L, mipLevel).rgb * NdotL;

            totalWeight += NdotL;
        }
    }

    color = color / totalWeight;

    SV_Target = vec4(color, 1.0);
}

// D = GGXTR(n, h, α) = α^2 / π((n·h)^2 * (α^2 - 1) + 1)^2
float GGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
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
