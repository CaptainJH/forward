
#include "hlslUtils.hlsl"
#include "shared.h"
#include "brdf.h"

static float M_PI = 3.141592f;
// -------------------------------------------------------------------------
//    Structures
// -------------------------------------------------------------------------
// The payload used for our indirect global illumination rays
struct IndirectRayPayload
{
	float3 color;    // The (returned) color in the ray's direction
	uint   rndSeed;  // Our random seed, so we pick uncorrelated RNGs along our ray
	uint   rayDepth; // What is the depth of our current ray?
};

// Payload for our shadow rays. 
struct ShadowRayPayload
{
	float visFactor;  // Will be 1.0 for fully lit, 0.0 for fully shadowed
};

struct VertexAttributes
{
	float3 position;
	float3 shadingNormal;
	float3 geometryNormal;
	float2 uv;
};

// -------------------------------------------------------------------------
//    Resources
// -------------------------------------------------------------------------

// Constant buffer with data needed for path tracing
cbuffer RaytracingDataCB : register(b0)
{
	RaytracingData gData;
}

cbuffer GIControl : register(b1)
{
	float3 g_LightPos;
	uint g_Enable_GI;
	float3 g_camPos;
	uint gMaxDepth;
	uint g_Enable_Local;
}

// Output buffer with accumulated and tonemapped image
RWTexture2D<float4> RTOutput						: register(u0);

Texture2D<float4> skyTexture						: register(t0);
Texture2D<float4> g_PosTex							: register(t1);
Texture2D<float4> g_NormalTex						: register(t2);
Texture2D<float4> g_DiffuseTex						: register(t3);
Texture2D<float4> g_RoughnessMetalnessTex			: register(t4);

// TLAS of our scene
RaytracingAccelerationStructure sceneBVH			: register(t0, space99);

// Bindless materials, geometry, and texture buffers (for all the scene geometry)
StructuredBuffer<MaterialData> materials			: register(t0, space1);
ByteAddressBuffer indices[MAX_INSTANCES_COUNT]		: register(t0, space2);
StructuredBuffer<VertexAttributes> vertices[MAX_INSTANCES_COUNT]		: register(t0, space3);
Texture2D<float4> textures[MAX_TEXTURES_COUNT]		: register(t0, space4);

// Texture Sampler
SamplerState linearSampler							: register(s0);

// -------------------------------------------------------------------------
//    Defines
// -------------------------------------------------------------------------

#define FLT_MAX 3.402823466e+38F

// Defines after how many bounces will be the Russian Roulette applied
#define MIN_BOUNCES 3

// Switches between two RNGs
#define USE_PCG 1

// Number of candidates used for resampling of analytical lights
#define RIS_CANDIDATES_LIGHTS 8

// Enable this to cast shadow rays for each candidate during resampling. This is expensive but increases quality
#define SHADOW_RAY_IN_RIS 0

// -------------------------------------------------------------------------
//    Utilities
// -------------------------------------------------------------------------

#define TONEMAP_GAMMA 1.0f

// Reinhard Tonemapper
float3 tonemap_reinhard(in float3 color)
{
   color *= 16;
   color = color/(1+color);
   float3 ret = pow(color, TONEMAP_GAMMA); // gamma
   return ret;
}

// Uncharted 2 Tonemapper
float3 tonemap_uncharted2(in float3 x)
{
    float A = 0.22;
    float B = 0.30;
    float C = 0.10;
    float D = 0.20;
    float E = 0.01;
    float F = 0.30;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 tonemap_uc2(in float3 color)
{
    float W = 11.2;

    color *= 16;  // Hardcoded Exposure Adjustment

    float exposure_bias = 2.0f;
    float3 curr = tonemap_uncharted2(exposure_bias*color);

    float3 white_scale = 1.0f/tonemap_uncharted2(W);
    float3 ccolor = curr*white_scale;

    float3 ret = pow(abs(ccolor), TONEMAP_GAMMA); // gamma

    return ret;
}

// Helpers to convert between linear and sRGB color spaces
float3 linearToSrgb(float3 linearColor)
{
	return float3(linearToSrgb(linearColor.x), linearToSrgb(linearColor.y), linearToSrgb(linearColor.z));
}

float3 srgbToLinear(float3 srgbColor)
{
	return float3(srgbToLinear(srgbColor.x), srgbToLinear(srgbColor.y), srgbToLinear(srgbColor.z));
}

// Helpers for octahedron encoding of normals
float2 octWrap(float2 v)
{
	return float2((1.0f - abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f), (1.0f - abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f));
}

float2 encodeNormalOctahedron(float3 n)
{
	float2 p = float2(n.x, n.y) * (1.0f / (abs(n.x) + abs(n.y) + abs(n.z)));
	p = (n.z < 0.0f) ? octWrap(p) : p;
	return p;
}

float3 decodeNormalOctahedron(float2 p)
{
	float3 n = float3(p.x, p.y, 1.0f - abs(p.x) - abs(p.y));
	float2 tmp = (n.z < 0.0f) ? octWrap(float2(n.x, n.y)) : float2(n.x, n.y);
	n.x = tmp.x;
	n.y = tmp.y;
	return normalize(n);
}

float4 encodeNormals(float3 geometryNormal, float3 shadingNormal) {
	return float4(encodeNormalOctahedron(geometryNormal), encodeNormalOctahedron(shadingNormal));
}

void decodeNormals(float4 encodedNormals, out float3 geometryNormal, out float3 shadingNormal) {
	geometryNormal = decodeNormalOctahedron(encodedNormals.xy);
	shadingNormal = decodeNormalOctahedron(encodedNormals.zw);
}

// -------------------------------------------------------------------------
//    Materials
// -------------------------------------------------------------------------

// Helper to read vertex indices of the triangle from index buffer
uint3 GetIndices(uint geometryID, uint triangleIndex)
{
	uint baseIndex = (triangleIndex * 3);
	int address = (baseIndex * 4);
	return indices[geometryID].Load3(address);
}

// Helper to interpolate vertex attributes at hit point from triangle vertices
VertexAttributes GetVertexAttributes(uint geometryID, uint triangleIndex, float3 barycentrics)
{
	// Get the triangle indices
	uint3 indices = GetIndices(geometryID, triangleIndex);
	VertexAttributes v = (VertexAttributes)0;
	float3 triangleVertices[3];

	// Interpolate the vertex attributes
	for (uint i = 0; i < 3; i++)
	{
		// Load and interpolate position and transform it to world space
		triangleVertices[i] = mul(ObjectToWorld3x4(), float4(vertices[geometryID][indices[i]].position, 1.0f)).xyz;
		v.position += triangleVertices[i] * barycentrics[i];

		// Load and interpolate normal
		v.shadingNormal += vertices[geometryID][indices[i]].shadingNormal * barycentrics[i];

		// Load and interpolate texture coordinates
		v.uv += vertices[geometryID][indices[i]].uv * barycentrics[i];
	}

	// Transform normal from local to world space
	v.shadingNormal = normalize(mul(ObjectToWorld3x4(), float4(v.shadingNormal, 0.0f)).xyz);

	// Calculate geometry normal from triangle vertices positions
	float3 edge20 = triangleVertices[2] - triangleVertices[0];
	float3 edge21 = triangleVertices[2] - triangleVertices[1];
	float3 edge10 = triangleVertices[1] - triangleVertices[0];
	v.geometryNormal = normalize(cross(edge20, edge10));

	return v;
}

// Loads material properties (including textures) for selected material
MaterialProperties loadMaterialProperties(uint materialID, float2 uvs) {
	MaterialProperties result = (MaterialProperties) 0;

	// Read base data
	MaterialData mData = materials[materialID];

	result.baseColor = mData.baseColor;
	result.emissive = mData.emissive;
	result.metalness = mData.metalness;
	result.roughness = mData.roughness;
	result.opacity = mData.opacity;
	
	// Load textures (using mip level 0)
	if (mData.baseColorTexIdx != INVALID_ID) {
		result.baseColor *= textures[mData.baseColorTexIdx].SampleLevel(linearSampler, uvs, 0.0f).rgb;
	}

	if (mData.emissiveTexIdx != INVALID_ID) {
		result.emissive *= textures[mData.emissiveTexIdx].SampleLevel(linearSampler, uvs, 0.0f).rgb;
	}

	if (mData.roughnessMetalnessTexIdx != INVALID_ID) {
		float3 occlusionRoughnessMetalness = textures[mData.roughnessMetalnessTexIdx].SampleLevel(linearSampler, uvs, 0.0f).rgb;
		result.metalness *= occlusionRoughnessMetalness.b;
		result.roughness *= occlusionRoughnessMetalness.g;
	}

	return result;
}

// -------------------------------------------------------------------------
//    Camera
// -------------------------------------------------------------------------

// Generates a primary ray for pixel given in NDC space using pinhole camera
RayDesc generatePinholeCameraRay(float2 pixel)
{
	// Setup the ray
	RayDesc ray;
	ray.Origin = gData.view[3].xyz;
	ray.TMin = 0.f;
	ray.TMax = FLT_MAX;

	// Extract the aspect ratio and field of view from the projection matrix
	float aspect = gData.proj[1][1] / gData.proj[0][0];
	float tanHalfFovY = 1.0f / gData.proj[1][1];

	// Compute the ray direction for this pixel
	ray.Direction = normalize(
		(pixel.x * gData.view[0].xyz * tanHalfFovY * aspect) -
		(pixel.y * gData.view[1].xyz * tanHalfFovY) +
			gData.view[2].xyz);

	return ray;
}

// -------------------------------------------------------------------------
//    Sky
// -------------------------------------------------------------------------

// Convert our world space direction to a (u,v) coord in a latitude-longitude spherical map
float2 wsVectorToLatLong(float3 dir)
{
	float3 p = normalize(dir);
	float u = (1.f + atan2(p.x, -p.z) * ONE_OVER_PI) * 0.5f;
	float v = acos(p.y) * ONE_OVER_PI;
	return float2(u, v);
}

float3 loadSkyValue(float3 rayDirection) {

	// Convert our ray direction to a (u,v) coordinate
	float2 uv = wsVectorToLatLong( rayDirection );
	float4 skyValue = skyTexture.SampleLevel(linearSampler, uv, 0.0f);

	// Load the sky value for given direction here, e.g. from environment map, procedural sky, etc.
	// Make sure to only account for sun once - either on the skybox or as an analytical light (if sun is included as explicit directional light, it shouldn't be on the skybox)
	return gData.skyIntensity * skyValue.rgb;
}

// Helper function to shoot shadow rays.  In: ray origin, dir, & min/max dist;  Out: 1=lit, 0=shadowed
float shadowRayVisibility( float3 origin, float3 direction, float minT, float maxT )
{
	// Setup our shadow ray
	RayDesc ray;
	ray.Origin = origin;        // Where does it start?
	ray.Direction = direction;  // What direction do we shoot it?
	ray.TMin = minT;            // The closest distance we'll count as a hit
	ray.TMax = maxT;            // The farthest distance we'll count as a hit

	ShadowRayPayload payload = (ShadowRayPayload) 0;

	// Query if anything is between the current point and the light
	TraceRay(sceneBVH, 
		     RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 
		     0xFF, 1, 0, 1, ray, payload);

	return payload.visFactor;
}

// A utility function to trace an indirect ray and return the color it sees.
//    -> Note:  This assumes the indirect hit programs and miss programs are index 1!
float3 shootIndirectRay(float3 rayOrigin, float3 rayDir, float minT, uint curPathLen, uint seed, uint curDepth)
{
	// Setup our indirect ray
	RayDesc rayColor;
	rayColor.Origin = rayOrigin;  // Where does it start?
	rayColor.Direction = rayDir;  // What direction do we shoot it?
	rayColor.TMin = minT;         // The closest distance we'll count as a hit
	rayColor.TMax = 1.0e38f;      // The farthest distance we'll count as a hit

	// Initialize the ray's payload data with black return color and the current rng seed
	IndirectRayPayload payload;
	payload.color = float3(0, 0, 0);
	payload.rndSeed = seed;
	payload.rayDepth = curDepth + 1;

	// Trace our ray to get a color in the indirect direction.  Use hit group #0 and miss shader #0
	TraceRay(sceneBVH, 0, 0xFF, 0, 0, 0, rayColor, payload);

	// Return the color we got from our ray
	return payload.color;
}

// Get a GGX half vector / microfacet normal, sampled according to the distribution computed by
//     the function ggxNormalDistribution() above.  
//
// When using this function to sample, the probability density is pdf = D * NdotH / (4 * HdotV)
float3 getGGXMicrofacet(inout uint randSeed, float roughness, float3 hitNorm)
{
	// Get our uniform random numbers
	float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

	// Get an orthonormal basis from the normal
	float3 B = getPerpendicularVector(hitNorm);
	float3 T = cross(B, hitNorm);

	// GGX NDF sampling
	float a2 = roughness * roughness;
	float cosThetaH = sqrt(max(0.0f, (1.0 - randVal.x) / ((a2 - 1.0) * randVal.x + 1)));
	float sinThetaH = sqrt(max(0.0f, 1.0f - cosThetaH * cosThetaH));
	float phiH = randVal.y * M_PI * 2.0f;

	// Get our GGX NDF sample (i.e., the half vector)
	return T * (sinThetaH * cos(phiH)) + B * (sinThetaH * sin(phiH)) + hitNorm * cosThetaH;
}

// Our material has have both a diffuse and a specular lobe.  
//     With what probability should we sample the diffuse one?
float probabilityToSampleDiffuse(float3 difColor, float3 specColor)
{
	float lumDiffuse = max(0.01f, luminance(difColor.rgb));
	float lumSpecular = max(0.01f, luminance(specColor.rgb));
	return lumDiffuse / (lumDiffuse + lumSpecular);
}

// The NDF for GGX, see Eqn 19 from 
//    http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
//
// This function can be used for "D" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float ggxNormalDistribution(float NdotH, float roughness)
{
	float a2 = roughness * roughness;
	float d = ((NdotH * a2 - NdotH) * NdotH + 1);
	return a2 / max(0.001f, (d * d * M_PI));
}

// This from Schlick 1994, modified as per Karas in SIGGRAPH 2013 "Physically Based Shading" course
//
// This function can be used for "G" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float ggxSchlickMaskingTerm(float NdotL, float NdotV, float roughness)
{
	// Karis notes they use alpha / 2 (or roughness^2 / 2)
	float k = roughness*roughness / 2;

	// Karis also notes they can use the following equation, but only for analytical lights
	//float k = (roughness + 1)*(roughness + 1) / 8; 

	// Compute G(v) and G(l).  These equations directly from Schlick 1994
	//     (Though note, Schlick's notation is cryptic and confusing.)
	float g_v = NdotV / (NdotV*(1 - k) + k);
	float g_l = NdotL / (NdotL*(1 - k) + k);

	// Return G(v) * G(l)
	return g_v * g_l;
}

// Traditional Schlick approximation to the Fresnel term (also from Schlick 1994)
//
// This function can be used for "F" in the Cook-Torrance model:  D*G*F / (4*NdotL*NdotV)
float3 schlickFresnel(float3 f0, float u)
{
	return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * pow(1.0f - u, 5.0f);
}

float3 ggxDirect(inout uint rndSeed, float3 hit, float3 N, float3 V, float3 dif, float3 spec, float rough)
{
	// We need to query our scene to find info about the current light
	float distToLight = distance(g_LightPos, hit);      // How far away is it?
	float3 L = normalize(g_LightPos - hit);         // What direction is it from our current pixel?

	// JHQ: ignore this light if N dot L is negative (i.e., light is behind the surface)
	if(dot(N, L) < 0.0f)
		return 0.0f;

	// Compute our lambertion term (L dot N)
	float NdotL = saturate(dot(N, L));

	// Shoot our ray.  Return 1.0 for lit, 0.0 for shadowed
	float shadowMult = shadowRayVisibility(hit, L, 0.001f, distToLight);

	// Compute half vectors and additional dot products for GGX
	float3 H = normalize(V + L);
	float NdotH = saturate(dot(N, H));
	float LdotH = saturate(dot(L, H));
	float NdotV = saturate(dot(N, V));

	// Evaluate terms for our GGX BRDF model
	float  D = ggxNormalDistribution(NdotH, rough);
	float  G = ggxSchlickMaskingTerm(NdotL, NdotV, rough);
	float3 F = schlickFresnel(spec, LdotH);

	// Evaluate the Cook-Torrance Microfacet BRDF model
	//     Cancel out NdotL here & the next eq. to avoid catastrophic numerical precision issues.
	float3 ggxTerm = D * G * F / (4 * NdotV /* * NdotL */);

	// Compute our final color (combining diffuse lobe plus specular GGX lobe)
	return shadowMult * ( /* NdotL * */ ggxTerm + NdotL * dif / M_PI);
}

float3 ggxIndirect(inout uint rndSeed, float3 hit, float3 N, float3 V, float3 dif, float3 spec, float rough, uint rayDepth)
{
	// We have to decide whether we sample our diffuse or specular/ggx lobe.
	float probDiffuse = probabilityToSampleDiffuse(dif, spec);
	float chooseDiffuse = (nextRand(rndSeed) < probDiffuse);

	// We'll need NdotV for both diffuse and specular...
	float NdotV = saturate(dot(N, V));

	// If we randomly selected to sample our diffuse lobe...
	if (chooseDiffuse)
	{
		// Shoot a randomly selected cosine-sampled diffuse ray.
		float3 L = getCosHemisphereSample(rndSeed, N);
		float3 bounceColor = shootIndirectRay(hit, L, 1.0f, 0, rndSeed, rayDepth);

		// // Check to make sure our randomly selected, normal mapped diffuse ray didn't go below the surface.
		// if (dot(noNormalN, L) <= 0.0f) bounceColor = float3(0, 0, 0);

		// Accumulate the color: (NdotL * incomingLight * dif / pi) 
		// Probability of sampling:  (NdotL / pi) * probDiffuse
		return bounceColor * dif / probDiffuse;
	}
	// Otherwise we randomly selected to sample our GGX lobe
	else
	{
		// Randomly sample the NDF to get a microfacet in our BRDF to reflect off of
		float3 H = getGGXMicrofacet(rndSeed, rough, N);

		// Compute the outgoing direction based on this (perfectly reflective) microfacet
		float3 L = normalize(2.f * dot(V, H) * H - V);

		// Compute our color by tracing a ray in this direction
		float3 bounceColor = shootIndirectRay(hit, L, 1.0f, 0, rndSeed, rayDepth);

		// Compute some dot products needed for shading
		float  NdotL = saturate(dot(N, L));
		float  NdotH = saturate(dot(N, H));
		float  LdotH = saturate(dot(L, H));

		// Evaluate our BRDF using a microfacet BRDF model
		float  D = ggxNormalDistribution(NdotH, rough);          // The GGX normal distribution
		float  G = ggxSchlickMaskingTerm(NdotL, NdotV, rough);   // Use Schlick's masking term approx
		float3 F = schlickFresnel(spec, LdotH);                  // Use Schlick's approx to Fresnel
		float3 ggxTerm = /*D * */ G * F / (4 * NdotL * NdotV);        // The Cook-Torrance microfacet BRDF

		// What's the probability of sampling vector H from getGGXMicrofacet()?
		// JHQ: cancel out D here & the ggxTerm above to avoid visual artifacts
		float  ggxProb = /*D * */ NdotH / (4 * LdotH);

		// Accumulate the color:  ggx-BRDF * incomingLight * NdotL / probability-of-sampling
		//    -> Should really simplify the math above.
		return NdotL * bounceColor * ggxTerm / (ggxProb * (1.0f - probDiffuse));
	}
}

// -------------------------------------------------------------------------
//    Raytracing shaders
// -------------------------------------------------------------------------

[shader("closesthit")]
void HitGroupIndirect_ClosestHit(inout IndirectRayPayload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	// At closest hit, we first load material and geometry ID packed into InstanceID 
	uint materialID;
	uint geometryID;
	unpackInstanceID(InstanceID(), materialID, geometryID);

	// Read hit point properties (position, normal, UVs, ...) from vertex buffer
	float3 barycentrics = float3((1.0f - attrib.barycentrics.x - attrib.barycentrics.y), attrib.barycentrics.x, attrib.barycentrics.y);
	VertexAttributes vertex = GetVertexAttributes(geometryID, PrimitiveIndex(), barycentrics);
	MaterialProperties material = loadMaterialProperties(materialID, vertex.uv);

	float3 worldPos = vertex.position;
	float3 worldNorm = vertex.shadingNormal;
	float3 V = normalize(g_camPos - worldPos);
	float3 difMatlColor = material.baseColor;
	float  roughness = material.roughness;
	float  metalness = material.metalness;
	float3 specularF0 = baseColorToSpecularF0(difMatlColor.rgb, metalness);

	payload.color = ggxDirect(payload.rndSeed, worldPos, worldNorm, V,
				                   difMatlColor.rgb, specularF0, roughness);

	// Do indirect illumination at this hit location (if we haven't traversed too far)
	if (payload.rayDepth < gMaxDepth)
	{
		// Use the same normal for the normal-mapped and non-normal mapped vectors... This means we could get light
		//     leaks at secondary surfaces with normal maps due to indirect rays going below the surface.  This
		//     isn't a huge issue, but this is a (TODO: fix)
		payload.color += ggxIndirect(payload.rndSeed, worldPos, worldNorm, V, 
									difMatlColor.rgb, specularF0, roughness, payload.rayDepth);
	}
}

[shader("anyhit")]
void HitGroupIndirect_AnyHit(inout IndirectRayPayload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	// At any hit, we test opacity and discard the hit if it's transparent
	if (alphaTestFails(attrib)) IgnoreHit();
}

[shader("miss")]
void IndirectMiss(inout IndirectRayPayload payload)
{
	payload.color = loadSkyValue(WorldRayDirection());
}

[shader("raygeneration")]
void ggxGIRayGen()
{
	uint2 LaunchIndex = DispatchRaysIndex().xy;
	uint2 LaunchDimensions = DispatchRaysDimensions().xy;

	// Load g-buffer data:  world-space position, normal, and diffuse color
	float4 worldPos     = g_PosTex[LaunchIndex];
	float3 worldNorm    = g_NormalTex[LaunchIndex].xyz;
	float3 difMatlColor = g_DiffuseTex[LaunchIndex].rgb;
	float  roughness 	= g_RoughnessMetalnessTex[LaunchIndex].g;
	float 	metalness 	= g_RoughnessMetalnessTex[LaunchIndex].b;

	float3 V = normalize(g_camPos - worldPos.xyz);

	// Initialize path tracing data
	float3 radiance = float3(0.0f, 0.0f, 0.0f);
	float3 throughput = float3(1.0f, 1.0f, 1.0f);
	float3 specularF0 = baseColorToSpecularF0(difMatlColor.rgb, metalness);
	
    {
		// On a miss, load the sky value and break out of the ray tracing loop
		if (worldPos.w == 0.0f) 
        {
			// Figure out pixel coordinates being raytraced
			float2 pixel = float2(DispatchRaysIndex().xy);
			const float2 resolution = float2(LaunchDimensions.xy);
			pixel = ((pixel + 0.5f) / resolution) * 2.0f - 1.0f;

			// Initialize ray to the primary ray
			RayDesc ray = generatePinholeCameraRay(pixel);
			radiance += throughput * loadSkyValue(ray.Direction);
		}
        else
        {
			// Initialize our random number generator
			uint randSeed = initRand(LaunchIndex.x + LaunchIndex.y * LaunchDimensions.x, gData.frameNumber, 16);

			if(g_Enable_Local > 0)
			{
				radiance += ggxDirect(randSeed, worldPos.xyz, worldNorm.xyz, V,
									difMatlColor.rgb, specularF0, roughness);
			}

			// Now do our indirect illumination
			if(g_Enable_GI > 0)
			{
				radiance += ggxIndirect(randSeed, worldPos.xyz, worldNorm.xyz, V, 
									difMatlColor.rgb, specularF0, roughness, 0);
			}
        }
	}

	// Copy accumulated result into output buffer (this one is only RGB8, so precision is not good enough for accumulation)
	// Note: Conversion from linear to sRGB here is not be necessary if conversion is applied later in the pipeline
	RTOutput[LaunchIndex] = float4(linearToSrgb(radiance * gData.exposureAdjustment), 1.0f);
}

// What code is executed when our ray misses all geometry?
[shader("miss")]
void ShadowMiss(inout ShadowRayPayload rayData)
{
	// If we miss all geometry, then the light is visibile
	rayData.visFactor = 1.0f;
}

// What code is executed when our ray hits a potentially transparent surface?
[shader("anyhit")]
void HitGroupShadow_ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
	if (alphaTestFails(attribs))
		IgnoreHit();
}
