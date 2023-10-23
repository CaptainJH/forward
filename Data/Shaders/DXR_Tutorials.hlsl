struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};

struct GBuffer 
{
    float4 color   : SV_Target0;
	float4 pos    : SV_Target1;
    float4 normal : SV_Target2;
};

Texture2D baseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState baseSampler : register(s0);
//-----------------------------------------------------------------------------
GBuffer Tutorial_3_PS( in VS_OUTPUT input ) : SV_Target
{
    GBuffer gBufOut;
	gBufOut.color = baseTexture.Sample(baseSampler, input.uv);
    gBufOut.normal = float4(normalTexture.Sample(baseSampler, input.uv).xyz, 1.0f);
    gBufOut.pos = float4(input.position.xyz / input.position.w, 1.0f);

	return gBufOut;
}