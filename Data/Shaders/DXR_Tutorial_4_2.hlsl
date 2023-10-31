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

struct Attributes
{
	float2 uv;
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

// Output buffer with accumulated and tonemapped image
RWTexture2D<float4> RTOutput						: register(u0);

Texture2D<float4> skyTexture						: register(t0);

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
 //    RNG
 // -------------------------------------------------------------------------

#if USE_PCG
	#define RngStateType uint4
#else
	#define RngStateType uint
#endif

// PCG random numbers generator
// Source: "Hash Functions for GPU Rendering" by Jarzynski & Olano
uint4 pcg4d(uint4 v)
{
	v = v * 1664525u + 1013904223u;

	v.x += v.y * v.w; 
	v.y += v.z * v.x; 
	v.z += v.x * v.y; 
	v.w += v.y * v.z;

	v = v ^ (v >> 16u);

	v.x += v.y * v.w; 
	v.y += v.z * v.x; 
	v.z += v.x * v.y; 
	v.w += v.y * v.z;

	return v;
}

// 32-bit Xorshift random number generator
uint xorshift(inout uint rngState)
{
	rngState ^= rngState << 13;
	rngState ^= rngState >> 17;
	rngState ^= rngState << 5;
	return rngState;
}

// Jenkins's "one at a time" hash function
uint jenkinsHash(uint x) {
	x += x << 10;
	x ^= x >> 6;
	x += x << 3;
	x ^= x >> 11;
	x += x << 15;
	return x;
}

// Converts unsigned integer into float int range <0; 1) by using 23 most significant bits for mantissa
float uintToFloat(uint x) {
	return asfloat(0x3f800000 | (x >> 9)) - 1.0f;
}

#if USE_PCG

// Initialize RNG for given pixel, and frame number (PCG version)
RngStateType initRNG(uint2 pixelCoords, uint2 resolution, uint frameNumber) {
	return RngStateType(pixelCoords.xy, frameNumber, 0); //< Seed for PCG uses a sequential sample number in 4th channel, which increments on every RNG call and starts from 0
}

// Return random float in <0; 1) range  (PCG version)
float rand(inout RngStateType rngState) {
	rngState.w++; //< Increment sample index
	return uintToFloat(pcg4d(rngState).x);
}

#else

// Initialize RNG for given pixel, and frame number (Xorshift-based version)
RngStateType initRNG(uint2 pixelCoords, uint2 resolution, uint frameNumber) {
	RngStateType seed = dot(pixelCoords, uint2(1, resolution.x)) ^ jenkinsHash(frameNumber);
	return jenkinsHash(seed);
}

// Return random float in <0; 1) range (Xorshift-based version)
float rand(inout RngStateType rngState) {
	return uintToFloat(xorshift(rngState));
}

#endif

// Maps integers to colors using the hash function (generates pseudo-random colors)
float3 hashAndColor(int i) {
	uint hash = jenkinsHash(i);
	float r = ((hash >> 0) & 0xFF) / 255.0f;
	float g = ((hash >> 8) & 0xFF) / 255.0f;
	float b = ((hash >> 16) & 0xFF) / 255.0f;
	return float3(r, g, b);
}

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

// Performs an opacity test in any hit shader for potential hit. Returns true if hit point is transparent and can be ignored
bool testOpacityAnyHit(Attributes attrib) {

	// Load material at hit point
	uint materialID;
	uint geometryID;
	unpackInstanceID(InstanceID(), materialID, geometryID);

	MaterialData mData = materials[materialID];
	float opacity = mData.opacity;
	
	// Also load the opacity texture if available
	if (mData.baseColorTexIdx != INVALID_ID) {
		float3 barycentrics = float3((1.0f - attrib.uv.x - attrib.uv.y), attrib.uv.x, attrib.uv.y);
		VertexAttributes vertex = GetVertexAttributes(geometryID, PrimitiveIndex(), barycentrics);
		opacity *= textures[mData.baseColorTexIdx].SampleLevel(linearSampler, vertex.uv, 0.0f).a;
	}

	// Decide whether this hit is opaque or not according to chosen alpha testing mode
	if (mData.alphaMode == ALPHA_MODE_MASK) {
		return (opacity < mData.alphaCutoff);
	} else {
		// Alpha blending mode
		float u = 0.5f; // If you want alpha blending, there should be a random u. Semi-transparent things are, however, better rendered using refracted rays with real IoR
		return (opacity < u);
	}
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

// Helper to generate aperture samples of the thin lens model
float2 getApertureSample(inout RngStateType rngState)
{
	// Generate a sample within a circular aperture. Other shapes can be implemented here
	// Using just. xy coordinates of hemisphere sample gives samples within a disk
	return sampleHemisphere(float2(rand(rngState), rand(rngState))).xy;
}

// Generates a primary ray for pixel given in NDC space using thin lens model (with depth of field)
RayDesc generateThinLensCameraRay(float2 pixel, inout RngStateType rngState)
{
	// First find the point in distance at which we want perfect focus 
	RayDesc ray = generatePinholeCameraRay(pixel);
	float3 focalPoint = ray.Origin + ray.Direction * gData.focusDistance;

	// Sample the aperture shape
	float2 apertureSample = getApertureSample(rngState) * gData.apertureSize;

	// Jitter the ray origin within camera plane using aperture sample
	float3 rightVector = gData.view[0].xyz;
	float3 upVector = gData.view[1].xyz;
	ray.Origin = ray.Origin + rightVector * apertureSample.x + upVector * apertureSample.y;

	// Set ray direction from jittered origin towards the focal point
	ray.Direction = normalize(focalPoint - ray.Origin);

	return ray;
}

// Generates primary ray either using pinhole camera (for zero-sized apertures) or thin lens model
RayDesc generatePrimaryRay(float2 posNdcXy, inout RngStateType rngState)
{
	if (gData.apertureSize == 0.0f)
		return generatePinholeCameraRay(posNdcXy);
	else
		return generateThinLensCameraRay(posNdcXy, rngState);
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

	HitInfo payload = (HitInfo) 0;

	// Query if anything is between the current point and the light
	TraceRay(sceneBVH, 
		     RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 
		     0xFF, 0, 0, 0, ray, payload);

	// Return our ray payload (which is 1 for visible, 0 for occluded)
	if (!payload.hasHit()) return 1.0f;
	return 0.0f;
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
	HitInfo payload = (HitInfo) 0; 

	// Trace our ray to get a color in the indirect direction.  Use hit group #1 and miss shader #1
	TraceRay(sceneBVH, 0, 0xFF, 0, 0, 0, rayColor, payload);

	if (payload.hasHit())
	{
		float3 gLightPos = float3(1.12f, 9.0f, 0.6f);
		float3 geometryNormal;
		float3 shadingNormal;
		decodeNormals(payload.encodedNormals, geometryNormal, shadingNormal);
		// We need to query our scene to find info about the current light
		float distToLight = distance(gLightPos, payload.hitPosition);
		float3 toLight = normalize(gLightPos - payload.hitPosition);

		// Compute our lambertion term (L dot N)
		float LdotN = saturate(dot(shadingNormal, toLight));

		// Shoot our shadow ray to our randomly selected light
		float shadowMult = shadowRayVisibility(payload.hitPosition, toLight, 1.0f, distToLight);

		MaterialProperties material = loadMaterialProperties(payload.materialID, payload.uvs);

		return shadowMult * material.baseColor * LdotN / M_PI;
	}
	else
	{
		return loadSkyValue(rayDir);
	}
}

// -------------------------------------------------------------------------
//    Raytracing shaders
// -------------------------------------------------------------------------

[shader("closesthit")]
void HitGroup_ClosestHit(inout HitInfo payload, Attributes attrib)
{
	// At closest hit, we first load material and geometry ID packed into InstanceID 
	uint materialID;
	uint geometryID;
	unpackInstanceID(InstanceID(), materialID, geometryID);

	// Read hit point properties (position, normal, UVs, ...) from vertex buffer
	float3 barycentrics = float3((1.0f - attrib.uv.x - attrib.uv.y), attrib.uv.x, attrib.uv.y);
	VertexAttributes vertex = GetVertexAttributes(geometryID, PrimitiveIndex(), barycentrics);

	// Encode hit point properties and material ID into payload
	payload.encodedNormals = encodeNormals(vertex.geometryNormal, vertex.shadingNormal);
	payload.hitPosition = vertex.position;
	payload.materialID = materialID;
	payload.uvs = vertex.uv;
}

[shader("anyhit")]
void HitGroup_AnyHit(inout HitInfo payload : SV_RayPayload, Attributes attrib : SV_IntersectionAttributes)
{
	// At any hit, we test opacity and discard the hit if it's transparent
	if (testOpacityAnyHit(attrib)) IgnoreHit();
}

[shader("miss")]
void Miss(inout HitInfo payload)
{
	// We indicate miss by storing invalid material ID in the payload
    payload.materialID = INVALID_ID;
}

[shader("raygeneration")]
void RayGen()
{
	uint2 LaunchIndex = DispatchRaysIndex().xy;
	uint2 LaunchDimensions = DispatchRaysDimensions().xy;

	// Initialize random numbers generator
	RngStateType rngState = initRNG(LaunchIndex, LaunchDimensions, gData.frameNumber);

	// Figure out pixel coordinates being raytraced
	float2 pixel = float2(DispatchRaysIndex().xy);
	const float2 resolution = float2(DispatchRaysDimensions().xy);

	pixel = (((pixel + 0.5f) / resolution) * 2.0f - 1.0f);

	// Initialize ray to the primary ray
	RayDesc ray = generatePrimaryRay(pixel, rngState);
	HitInfo payload = (HitInfo) 0;

	// Initialize path tracing data
	float3 radiance = float3(0.0f, 0.0f, 0.0f);
	float3 throughput = float3(1.0f, 1.0f, 1.0f);
	
    {
		// Trace the ray
		TraceRay(
			sceneBVH,
			RAY_FLAG_NONE,
			0xFF,
			STANDARD_RAY_INDEX,
			0,
			STANDARD_RAY_INDEX,
			ray,
			payload);

		// On a miss, load the sky value and break out of the ray tracing loop
		if (!payload.hasHit()) 
        {
			radiance += throughput * loadSkyValue(ray.Direction);
		}
        else
        {
			// Initialize our random number generator
			uint randSeed = initRand(LaunchIndex.x + LaunchIndex.y * LaunchDimensions.x, 1, 16);

            // Decode normals and flip them towards the incident ray direction (needed for backfacing triangles)
            float3 geometryNormal;
            float3 shadingNormal;
            decodeNormals(payload.encodedNormals, geometryNormal, shadingNormal);

            float3 V = -ray.Direction;
            if (dot(geometryNormal, V) < 0.0f) geometryNormal = -geometryNormal;
            if (dot(geometryNormal, shadingNormal) < 0.0f) shadingNormal = -shadingNormal;

			float3 gLightPos = float3(1.12f, 9.0f, 0.6f);
			// We need to query our scene to find info about the current light
			float distToLight = distance(gLightPos, payload.hitPosition);      // How far away is it?
			float3 toLight = normalize(gLightPos - payload.hitPosition);         // What direction is it from our current pixel?

			// Compute our lambertion term (L dot N)
			float LdotN = saturate(dot(shadingNormal, toLight));

            // Load material properties at the hit point
            MaterialProperties material = loadMaterialProperties(payload.materialID, payload.uvs);

			// Shoot our ray.  Return 1.0 for lit, 0.0 for shadowed
			float shadowMult = shadowRayVisibility(payload.hitPosition, toLight, 1.0f, distToLight);

            // Account for emissive surfaces
            radiance += shadowMult * throughput * material.baseColor * LdotN / M_PI;

			// Now do our indirect illumination
			{
				// Select a random direction for our diffuse interreflection ray.
				float3 bounceDir = getCosHemisphereSample(randSeed, shadingNormal);      // Use cosine sampling

				// Get NdotL for our selected ray direction
				float NdotL = saturate(dot(shadingNormal, bounceDir));

				// Shoot our indirect global illumination ray
				float3 bounceColor = shootIndirectRay(payload.hitPosition, bounceDir, 1.0f, randSeed);

				// Probability of selecting this ray ( cos/pi for cosine sampling, 1/2pi for uniform sampling )
				float sampleProb = NdotL / M_PI;

				// Accumulate the color.  For performance, terms could (and should) be cancelled here.
				radiance += (NdotL * bounceColor * material.baseColor / M_PI) / sampleProb;
			}
        }
	}

	// Copy accumulated result into output buffer (this one is only RGB8, so precision is not good enough for accumulation)
	// Note: Conversion from linear to sRGB here is not be necessary if conversion is applied later in the pipeline
	RTOutput[LaunchIndex] = float4(linearToSrgb(radiance * gData.exposureAdjustment), 1.0f);
}
