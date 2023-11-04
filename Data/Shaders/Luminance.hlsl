
RWTexture2D<float4> OutputTexture   : register(u0);   
Texture2D<float4> SrcTexture        : register(t0);

// Shared memory
groupshared float3 SharedMem[64];

struct ComputeShaderInput
{
    uint3 GroupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
    uint3 GroupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
    uint3 DispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
    uint  GroupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

[numthreads(8, 8, 1)]
void LuminanceMain(ComputeShaderInput input)
{
    uint2 xy = input.DispatchThreadID.xy * 15;
    float3 value = float3(0, 0, 0);
    for(uint x = 0; x < 15; ++x)
    {
        for(uint y = 0; y < 15; ++y)
        {
            value += SrcTexture[xy + uint2(x, y)].rgb;
        }
    }

    uint ThreadIdx = input.GroupThreadID.y * 8 + input.GroupThreadID.x;
    SharedMem[ThreadIdx] = value / (15 * 15);

    // Wait for all threads in the group to finish loading
    GroupMemoryBarrierWithGroupSync();

    for(uint s = 64 / 2; s > 0; s >>= 1)
	{
		if(ThreadIdx < s)
			SharedMem[ThreadIdx] += SharedMem[ThreadIdx + s];
		GroupMemoryBarrierWithGroupSync();
	}

    // Have the first thread write out to the output texture
	if(ThreadIdx == 0)
		OutputTexture[input.GroupID.xy] =  float4(SharedMem[0] / 64, 1.0f);
}

float luminance(float3 rgb)
{
	return dot(rgb, float3(0.2126f, 0.7152f, 0.0722f));
}

// Shared memory
groupshared float3 SharedMem2[16 * 9];
[numthreads(16, 9, 1)]
void LuminanceMain2(ComputeShaderInput input)
{
    uint2 xy = input.DispatchThreadID.xy;

    uint ThreadIdx = input.GroupThreadID.y * 16 + input.GroupThreadID.x;
    SharedMem2[ThreadIdx] = SrcTexture[xy].rgb;

    // Wait for all threads in the group to finish loading
    GroupMemoryBarrierWithGroupSync();

    for(uint s = 1; s < 9; ++s)
	{
		if(xy.y == 0)
			SharedMem2[ThreadIdx] += SharedMem2[ThreadIdx + s * 16];
		GroupMemoryBarrierWithGroupSync();
	}

    // Have the first thread write out to the output texture
	if(ThreadIdx == 0)
    {
        float3 value = float3(0, 0, 0);
        for(uint s = 0; s < 16; ++s)
            value += SharedMem2[s];
        value /= 16 * 9;
        OutputTexture[uint2(0, 0)] = float4(value, luminance(value));
    }
}