
RWTexture2D<float4> OutputTexture : register(u0);


[numthreads(8, 8, 1)]
void BakerMain(uint3 DTid : SV_DispatchThreadID)
{

	OutputTexture[DTid.xy] = float4(1, 0, 0, 1);
}