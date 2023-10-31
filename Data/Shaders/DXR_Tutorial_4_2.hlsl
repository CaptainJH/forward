/* Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// This is a code sample accompanying "The Reference Path Tracer" chapter in Ray Tracing Gems 2
// v1.0, April 2021

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
}

// Output buffer with accumulated and tonemapped image
RWTexture2D<float4> RTOutput						: register(u0);
RWTexture2D<float4> accumulationBuffer				: register(u1);

Texture2D<float4> skyTexture						: register(t0);
Texture2D<float4> g_PosTex							: register(t1);
Texture2D<float4> g_NormalTex						: register(t2);
Texture2D<float4> g_DiffuseTex						: register(t3);

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
float3 shootIndirectRay(float3 rayOrigin, float3 rayDir, float minT, uint seed)
{
	// Setup shadow ray
	RayDesc rayColor;
	rayColor.Origin = rayOrigin;  // Where does it start?
	rayColor.Direction = rayDir;  // What direction do we shoot it?
	rayColor.TMin = minT;         // The closest distance we'll count as a hit
	rayColor.TMax = 1.0e38f;      // The farthest distance we'll count as a hit

	// Initialize the ray's payload data with black return color and the current rng seed
	IndirectRayPayload payload = (IndirectRayPayload) 0; 

	// Trace our ray to get a color in the indirect direction.  Use hit group #1 and miss shader #1
	TraceRay(sceneBVH, 0, 0xFF, 0, 0, 0, rayColor, payload);

	return payload.color;
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

	// We need to query our scene to find info about the current light
	float distToLight = distance(g_LightPos, vertex.position);
	float3 toLight = normalize(g_LightPos - vertex.position);

	// Compute our lambertion term (L dot N)
	float LdotN = saturate(dot(vertex.shadingNormal, toLight));

	// Shoot our shadow ray to our randomly selected light
	float shadowMult = shadowRayVisibility(vertex.position, toLight, RayTMin(), distToLight);

	MaterialProperties material = loadMaterialProperties(materialID, vertex.uv);

	payload.color = shadowMult * material.baseColor * LdotN / M_PI;
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
void SimpleDiffuseGIRayGen()
{
	uint2 LaunchIndex = DispatchRaysIndex().xy;
	uint2 LaunchDimensions = DispatchRaysDimensions().xy;

	// Load g-buffer data:  world-space position, normal, and diffuse color
	float4 worldPos     = g_PosTex[LaunchIndex];
	float3 worldNorm    = g_NormalTex[LaunchIndex].xyz;
	float3 difMatlColor = g_DiffuseTex[LaunchIndex].rgb;

	// Initialize path tracing data
	float3 radiance = float3(0.0f, 0.0f, 0.0f);
	float3 throughput = float3(1.0f, 1.0f, 1.0f);
	
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

			// We need to query our scene to find info about the current light
			float distToLight = distance(g_LightPos, worldPos);      // How far away is it?
			float3 toLight = normalize(g_LightPos - worldPos);         // What direction is it from our current pixel?

			// Compute our lambertion term (L dot N)
			float LdotN = saturate(dot(worldNorm, toLight));

			// Shoot our ray.  Return 1.0 for lit, 0.0 for shadowed
			float shadowMult = shadowRayVisibility(worldPos, toLight, 1.0f, distToLight);

            // Account for emissive surfaces
            radiance += shadowMult * throughput * difMatlColor * LdotN / M_PI;

			// Now do our indirect illumination
			if(g_Enable_GI > 0)
			{
				// Select a random direction for our diffuse interreflection ray.
				float3 bounceDir = getCosHemisphereSample(randSeed, worldNorm);      // Use cosine sampling

				// Get NdotL for our selected ray direction
				float NdotL = saturate(dot(worldNorm, bounceDir));

				// Shoot our indirect global illumination ray
				float3 bounceColor = shootIndirectRay(worldPos, bounceDir, 1.0f, randSeed);

				// Probability of selecting this ray ( cos/pi for cosine sampling, 1/2pi for uniform sampling )
				float sampleProb = NdotL / M_PI;

				// Accumulate the color.  For performance, terms could (and should) be cancelled here.
				radiance += (NdotL * bounceColor * difMatlColor / M_PI) / sampleProb;
			}
        }
	}

	// Copy accumulated result into output buffer (this one is only RGB8, so precision is not good enough for accumulation)
	// Note: Conversion from linear to sRGB here is not be necessary if conversion is applied later in the pipeline
	float4 v = float4(linearToSrgb(radiance * gData.exposureAdjustment), 1.0f);

    if(gData.accumulatedFrames > 0)
        accumulationBuffer[LaunchIndex] = accumulationBuffer[LaunchIndex] + v;
    else
        accumulationBuffer[LaunchIndex] = v;
    RTOutput[LaunchIndex] = accumulationBuffer[LaunchIndex] / (gData.accumulatedFrames + 1);
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
