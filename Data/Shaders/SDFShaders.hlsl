//-----------------------------------------------------------------------------
cbuffer SDFParams
{
	float4 screenParams;
	float4 camPos;
	matrix WorldMatrix;
	matrix ViewProjMatrix;
	matrix ViewProjInverse;
};

Texture3D<float> tex0: register( t0 );
SamplerState s0: register( s0 );

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float4 position : POSITION;
	float3 color : COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 color : COLOR;
};

float4 ConvertFromScreenCoordToWorldSpaceCoord(float2 screenPos)
{
	float width = screenParams.x;
	float height = screenParams.y;

	float x = 2.0f * screenPos.x / width - 1;
	float y = -2.0f * screenPos.y / height + 1;
	
	float4 posScn = float4(x, y, 0, 1);
	float4 posW = mul(posScn, ViewProjInverse);
	posW = posW / posW.w;

	return posW;
}

//-----------------------------------------------------------------------------
VS_OUTPUT VSMainQuad( in VS_INPUT v, uint vid : SV_VertexID )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = v.position;
	if(vid == 0)
		o.color.xyz = float3(0, 0, 0);
	else if(vid == 1)
		o.color.xyz = float3(1, 0, 0);
	else if(vid == 2)
		o.color.xyz = float3(0, 1, 1);
	else if(vid == 3)
		o.color.xyz = float3(1, 1, 1);

	return o;
}

float3 checkPixel(float2 screenPos, float4 posScn)
{
	float width = screenParams.x;
	float height = screenParams.y;

	float x = 2.0f * screenPos.x / width - 1;
	float y = -2.0f * screenPos.y / height + 1;

	return float3(abs(x - posScn.x), abs(y - posScn.y), 0.0f);	
}

float3 PSMainQuad( in VS_OUTPUT input ) : SV_Target
{
	float4 pos = ConvertFromScreenCoordToWorldSpaceCoord(input.position.xy);
	float4 posScn = mul(pos, ViewProjMatrix);
	posScn = posScn / posScn.w;

	return checkPixel(input.position.xy, posScn);
}