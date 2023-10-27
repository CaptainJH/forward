#include "hlslUtils.hlsl"

// Payload for our shadow rays. 
struct ShadowRayPayload
{
	float visFactor;  // Will be 1.0 for fully lit, 0.0 for fully shadowed
};

// A constant buffer we'll populate from our C++ code 
cbuffer RayGenCB : register( b0 )
{
    float3 gLightIntensity;
    float3 gLightPos;
	float gMinT;      // Min distance to start a ray to avoid self-occlusion
}

// Input and out textures that need to be set by the C++ code
Texture2D<float4>   gPos            : register(t0);     // G-buffer world-space position
Texture2D<float4>   gNorm           : register(t1);     // G-buffer world-space normal
Texture2D<float4>   gDiffuseMatl    : register(t2);     // G-buffer diffuse material (RGB) and opacity (A)
RWTexture2D<float4> gOutput         : register(u0);     // Output to store shaded result

// TLAS of our scene
RaytracingAccelerationStructure sceneBVH			: register(t0, space99);

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

// How do we shade our g-buffer and generate shadow rays?
[shader("raygeneration")]
void LambertShadowsRayGen()
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
			float shadowMult = 1.0f;//shadowRayVisibility(worldPos.xyz, toLight, gMinT, distToLight);

			// Accumulate our Lambertian shading color
			shadeColor += shadowMult * LdotN; 
		}

		// Modulate based on the physically based Lambertian term (albedo/pi)
		// shadeColor *= difMatlColor.rgb / 3.141592f;
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