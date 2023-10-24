
// A separate file defining the mostly mathematical utilities we use below
#include "hlslUtils.hlsl"

// Payload for our AO rays.
struct AORayPayload
{
	float aoValue;  // Store 0 if we hit a surface, 1 if we miss all surfaces
};

// A constant buffer we'll populate from our C++ code 
cbuffer RayGenCB : register(b0)
{
	float gAORadius;
	uint  gFrameCount;
	float gMinT;
	uint  gNumRays;
}

// Input and out textures that need to be set by the C++ code
Texture2D<float4> gPos          : register(t0);
Texture2D<float4> gNorm         : register(t1);
RWTexture2D<float4> gOutput     : register(u0);

// TLAS of our scene
RaytracingAccelerationStructure sceneBVH			: register(t0, space99);

// A wrapper function that encapsulates shooting an ambient occlusion ray query
float shootAmbientOcclusionRay( float3 orig, float3 dir, float minT, float maxT )
{
	// Setup ambient occlusion ray and payload
	AORayPayload  rayPayload = { 0.0f };  // Specified value is returned if we hit a surface
	RayDesc       rayAO;
	rayAO.Origin    = orig;               // Where does our ray start?
	rayAO.Direction = dir;                // What direction does our ray go?
	rayAO.TMin      = minT;               // Min distance to detect an intersection
	rayAO.TMax      = maxT;               // Max distance to detect an intersection

	// Trace our ray.  Ray stops after it's first definite hit; never execute closest hit shader
	TraceRay(sceneBVH,
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
		0xFF, 0, 0, 0, rayAO, rayPayload);

	// Copy our AO value out of the ray payload.
	return rayPayload.aoValue;
}


// How do we generate the rays that we trace?
[shader("raygeneration")]
void AoRayGen()
{
	// Where is this thread's ray on screen?
	uint2 launchIndex = DispatchRaysIndex().xy;
	uint2 launchDim   = DispatchRaysDimensions().xy;

	// Initialize a random seed, per-pixel, based on a screen position and temporally varying count
	uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, gFrameCount, 16);

	// Load the position and normal from our g-buffer
	float4 worldPos = gPos[launchIndex];
	float4 worldNorm = gNorm[launchIndex];

	// Default ambient occlusion value if we hit the background
	float ambientOcclusion = float(gNumRays);

	// Our G-buffer stores 0 in worldPos.w for background pixels; only shoot an AO ray elsewhere
	if (worldPos.w != 0.0f)  
	{
		// Start accumulating from zero if we don't hit the background
		ambientOcclusion = 0.0f;

		for (int i = 0; i < gNumRays; i++)
		{

			// Sample cosine-weighted hemisphere around surface normal to pick a random ray direction
			float3 worldDir = getCosHemisphereSample(randSeed, worldNorm.xyz);

			// Shoot our ambient occlusion ray and update the value we'll output with the result
			ambientOcclusion += shootAmbientOcclusionRay(worldPos.xyz, worldDir, gMinT, gAORadius);
		}
	}
	
	// Save out our AO color
	float aoColor = ambientOcclusion / float(gNumRays);
	gOutput[launchIndex] = float4(aoColor, aoColor, aoColor, 1.0f);
}



// What code is executed when our ray misses all geometry?
[shader("miss")]
void AoMiss(inout AORayPayload rayData)
{
	// Our ambient occlusion value is 1 if we hit nothing.
	rayData.aoValue = 1.0f;
}

// What code is executed when our ray hits a potentially transparent surface?
[shader("anyhit")]
void HitGroup_AoAnyHit(inout AORayPayload rayData, BuiltInTriangleIntersectionAttributes attribs)
{
	// Is this a transparent part of the surface?  If so, ignore this hit
	if (alphaTestFails(attribs))
		IgnoreHit();
}
