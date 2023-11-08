
struct VS_INPUT
{
	float4 position : POSITION;
	float4 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 posWorld : POSITION;
	float4 normalWorld : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};

struct GBuffer 
{
    float4 color                : SV_Target0;
	float4 pos                  : SV_Target1;
    float4 normal               : SV_Target2;
};

cbuffer Transforms : register(b0)
{
	// matrix ViewProjMatrix;
	matrix WorldMatrix;	
	matrix InverseTransposeWorldMatrix;
};

Texture2D baseTexture       : register(t0);
Texture2D normalTexture     : register(t1);
SamplerState baseSampler    : register(s0);
//-----------------------------------------------------------------------------

float2 uv2quad(float2 uv)
{
    float2 uvTemp = frac(uv) * 2.0f - 1.0f;
    return float2(uvTemp.x, -uvTemp.y);
}

VS_OUTPUT VSMain( in VS_INPUT v, uint vid : SV_VertexID )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = float4(uv2quad(v.uv), 0, 1);
    o.posWorld = mul(v.position, WorldMatrix);
    o.normalWorld = mul(v.normal, InverseTransposeWorldMatrix);
    o.uv = v.uv;
    o.color = float4(1, 0, 0, 1);

	return o;
}

GBuffer PSMain( in VS_OUTPUT input )
{
    GBuffer gBufOut;
	gBufOut.color = baseTexture.Sample(baseSampler, input.uv);
    gBufOut.pos = input.posWorld;
    gBufOut.normal = input.normalWorld;
    return gBufOut;
}