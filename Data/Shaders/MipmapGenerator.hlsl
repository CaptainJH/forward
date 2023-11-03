
RWTexture2D<float4> MipmapTexture   : register(u0);   
Texture2D<float4> SrcTexture        : register(t0);

[numthreads(1, 1, 1)]
void MipmapGenerator(uint3 DTid : SV_DispatchThreadID)
{
    float4 v = float4(0, 0, 0, 0);
    float2 xy = DTid.xy * 2;
    v += SrcTexture[xy + uint2(0, 0)];
    v += SrcTexture[xy + uint2(1, 0)];
    v += SrcTexture[xy + uint2(0, 1)];
    v += SrcTexture[xy + uint2(1, 1)];
    MipmapTexture[DTid.xy] = v / 4;
}