#include "hlslUtils.hlsl"
#include "shared.h"
#include "brdf.h"

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

// A constant buffer we'll populate from our C++ code 
cbuffer RayGenCB : register( b0 )
{
    float3 gLightIntensity;
	float gMinT;      // Min distance to start a ray to avoid self-occlusion
    float3 gLightPos;
    uint gFrameCount;
}

// Input and out textures that need to be set by the C++ code
Texture2D<float4>   gPos            : register(t0);     // G-buffer world-space position
Texture2D<float4>   gNorm           : register(t1);     // G-buffer world-space normal
Texture2D<float4>   gDiffuseMatl    : register(t2);     // G-buffer diffuse material (RGB) and opacity (A)
Texture2D<float4>   gEnvMap         : register(t3);     // Environment map
RWTexture2D<float4> gOutput         : register(u0);     // Output to store shaded result

// TLAS of our scene
RaytracingAccelerationStructure sceneBVH			: register(t0, space99);
// Bindless materials, geometry, and texture buffers (for all the scene geometry)
StructuredBuffer<MaterialData> materials			: register(t0, space1);
ByteAddressBuffer indices[MAX_INSTANCES_COUNT]		: register(t0, space2);
StructuredBuffer<VertexAttributes> vertices[MAX_INSTANCES_COUNT]		: register(t0, space3);
Texture2D<float4> textures[MAX_TEXTURES_COUNT]		: register(t0, space4);

// Texture Sampler
SamplerState linearSampler							: register(s0);

static float M_PI = 3.141592f;

// The payload used for our indirect global illumination rays
struct IndirectRayPayload
{
	float3 color;    // The (returned) color in the ray's direction
};


// Helper function to shoot shadow rays.  In: ray origin, dir, & min/max dist;  Out: 1=lit, 0=shadowed
float shadowRayVisibility( float3 origin, float3 direction, float minT, float maxT )
{
	// Setup our shadow ray
	RayDesc ray;
	ray.Origin = origin;        // Where does it start?
	ray.Direction = direction;  // What direction do we shoot it?
	ray.TMin = minT;            // The closest distance we'll count as a hit
	ray.TMax = maxT;            // The farthest distance we'll count as a hit

	// Our shadow rays are *assumed* to hit geometry; this miss shader changes this to 1.0 for "visible"
	ShadowRayPayload payload = { 0.0f };   

	// Query if anything is between the current point and the light
	TraceRay(sceneBVH, 
		     RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 
		     0xFF, 0, 0, 0, ray, payload);

	// Return our ray payload (which is 1 for visible, 0 for occluded)
	return payload.visFactor;
}

// A utility function to trace an idirect ray and return the color it sees.
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
	IndirectRayPayload payload;
	payload.color = float3(0, 0, 0);  

	// Trace our ray to get a color in the indirect direction.  Use hit group #1 and miss shader #1
	TraceRay(sceneBVH, 0, 0xFF, 1, 0, 1, rayColor, payload);

	// Return the color we got from our ray
	return payload.color;
}

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

// Convert our world space direction to a (u,v) coord in a latitude-longitude spherical map
float2 wsVectorToLatLong(float3 dir)
{
	float3 p = normalize(dir);
	float u = (1.f + atan2(p.x, -p.z) * ONE_OVER_PI) * 0.5f;
	float v = acos(p.y) * ONE_OVER_PI;
	return float2(u, v);
}

// How do we shade our g-buffer and generate shadow rays?
[shader("raygeneration")]
void SimpleDiffuseGIRayGen()
{
	// Get our pixel's position on the screen
	uint2 launchIndex = DispatchRaysIndex().xy;
	uint2 launchDim   = DispatchRaysDimensions().xy;

	// Load g-buffer data:  world-space position, normal, and diffuse color
	float4 worldPos     = gPos[launchIndex];
	float4 worldNorm    = gNorm[launchIndex];
	float4 difMatlColor = gDiffuseMatl[launchIndex];

	// If we don't hit any geometry, our difuse material contains our background color.
	float3 shadeColor = difMatlColor.rgb;

	// Initialize our random number generator
	// uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, gFrameCount, 16);

	// Our camera sees the background if worldPos.w is 0, only do diffuse shading
	if (worldPos.w != 0.0f)
	{
		// We're going to accumulate contributions from multiple lights, so zero out our sum
		shadeColor = float3(0.0, 0.0, 0.0);

		// Lighting calculation:
		{
			// We need to query our scene to find info about the current light
			float distToLight = distance(gLightPos, worldPos.xyz);      // How far away is it?
			float3 toLight = normalize(gLightPos - worldPos.xyz);         // What direction is it from our current pixel?

			// Compute our lambertion term (L dot N)
			float LdotN = saturate(dot(worldNorm.xyz, toLight));

			// Shoot our ray.  Return 1.0 for lit, 0.0 for shadowed
			float shadowMult = shadowRayVisibility(worldPos.xyz, toLight, gMinT, distToLight);

			// Accumulate our Lambertian shading color
			shadeColor += shadowMult * LdotN * gLightIntensity; 
		}

		// Modulate based on the physically based Lambertian term (albedo/pi)
		shadeColor *= difMatlColor.rgb / 3.141592f;

        // // Now do our indirect illumination
		// {
		// 	// Select a random direction for our diffuse interreflection ray.
		// 	float3 bounceDir = getCosHemisphereSample(randSeed, worldNorm.xyz);      // Use cosine sampling

		// 	// Get NdotL for our selected ray direction
		// 	float NdotL = saturate(dot(worldNorm.xyz, bounceDir));

		// 	// Shoot our indirect global illumination ray
		// 	float3 bounceColor = shootIndirectRay(worldPos.xyz, bounceDir, gMinT, randSeed);


		// 	// Probability of selecting this ray ( cos/pi for cosine sampling, 1/2pi for uniform sampling )
		// 	float sampleProb = NdotL / M_PI;

		// 	// Accumulate the color.  For performance, terms could (and should) be cancelled here.
		// 	shadeColor += (NdotL * bounceColor * difMatlColor.rgb / M_PI) / sampleProb;
		// }
	}

	// Save out our final shaded
	gOutput[launchIndex] = float4(shadeColor, 1.0f);
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
void HitGroup_ShadowAnyHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
	if (alphaTestFails(attribs))
		IgnoreHit();
}

// What code is executed when we have a new closest hitpoint?
[shader("closesthit")]
void HitGroup_ShadowClosestHit(inout ShadowRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
}

// What code is executed when our ray misses all geometry?
[shader("miss")]
void IndirectMiss(inout IndirectRayPayload rayData)
{
	// Convert our ray direction to a (u,v) coordinate
	float2 uv = wsVectorToLatLong( WorldRayDirection() );
	float4 skyValue = gEnvMap.SampleLevel(linearSampler, uv, 0.0f);

	// Load our background color, then store it into our ray payload
	rayData.color = skyValue.rgb;
}

// What code is executed when our ray hits a potentially transparent surface?
[shader("anyhit")]
void HitGroupIndirect_IndirectAnyHit(inout IndirectRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
	if (alphaTestFails(attribs))
		IgnoreHit();
}

// What code is executed when we have a new closest hitpoint?   Well, pick a random light,
//    shoot a shadow ray to that light, and shade using diffuse shading.
[shader("closesthit")]
void HitGroupIndirect_IndirectClosestHit(inout IndirectRayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// At closest hit, we first load material and geometry ID packed into InstanceID 
	uint materialID;
	uint geometryID;
	unpackInstanceID(InstanceID(), materialID, geometryID);

	// Read hit point properties (position, normal, UVs, ...) from vertex buffer
	float3 barycentrics = float3((1.0f - attribs.barycentrics.x - attribs.barycentrics.y), attribs.barycentrics.x, attribs.barycentrics.y);
	VertexAttributes vertex = GetVertexAttributes(geometryID, PrimitiveIndex(), barycentrics);

    float distToLight = distance(gLightPos, vertex.position);
    float3 toLight = normalize(gLightPos - vertex.position);

	// Compute our lambertion term (L dot N)
	float LdotN = saturate(dot(vertex.geometryNormal, toLight));

	// Shoot our shadow ray to our randomly selected light
	float shadowMult = shadowRayVisibility(vertex.position, toLight, RayTMin(), distToLight);

	// Return the Lambertian shading color using the physically based Lambertian term (albedo / pi)
    MaterialProperties material = loadMaterialProperties(materialID, vertex.uv);
	rayData.color = shadowMult * LdotN * gLightIntensity * material.baseColor / M_PI;
}