#include "BasicShader.hlsl"

struct GBuffer 
{
    float4 color                : SV_Target0;
	float4 pos                  : SV_Target1;
    float4 normal               : SV_Target2;
    float4 roughnessMetalness   : SV_Target3;
};

Texture2D normalTexture             : register(t1);
Texture2D roughnessMetalnessTexture : register(t2);
//-----------------------------------------------------------------------------
// Calculates a cotangent frame without precomputed tangents by Christian Schüler
// ported from GLSL to HLSL; see: http://www.thetenthplanet.de/archives/1180
float3x3 calcWorldSpaceCotangentFrame(float3 wsNormal, float3 wsInvViewDir, float2 tsCoord)
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx(wsInvViewDir);
    float3 dp2 = ddy(wsInvViewDir);
    float2 duv1 = ddx(tsCoord);
    float2 duv2 = ddy(tsCoord);
 
    // solve the linear system
    float3 dp2perp = cross(dp2, wsNormal);
    float3 dp1perp = cross(wsNormal, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct and return a scale-invariant cotangent frame
    float invmax = rsqrt(max(dot(T,T), dot(B,B)));
    return float3x3(T * invmax, B * invmax, wsNormal);
}

GBuffer Tutorial_3_PS( in VS_OUTPUT input )
{
    GBuffer gBufOut;
	gBufOut.color = baseTexture.Sample(baseSampler, input.uv);
    gBufOut.pos = float4(input.posWorld, 1.0f);
    gBufOut.roughnessMetalness = roughnessMetalnessTexture.Sample(baseSampler, input.uv);

    float3 sampledNormal = normalTexture.Sample(baseSampler, input.uv).xyz * 2 - 1.0f;
    float3x3 cotangentFrame = calcWorldSpaceCotangentFrame(input.normalWorld, normalize(CamPosition - input.posWorld), input.uv);
    float3 normalWorld = mul(sampledNormal, cotangentFrame);
    gBufOut.normal = float4(normalize(normalWorld), 1.0f);

	return gBufOut;
}