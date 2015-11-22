//-----------------------------------------------------------------------------
cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	matrix WorldViewProjMatrixLight;
};

Texture2D tex0: register( t0 );
SamplerState s0: register( s0 );

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float4 positionLight : PositionLightSpace;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain( in VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = v.color;
	o.positionLight = mul(v.position, WorldViewProjMatrixLight);

	return o;
}


//-----------------------------------------------------------------------------
float4 PSMain( in VS_OUTPUT input ) : SV_Target
{
	float4 ambient = float4(0.1, 0.1, 0.1, 1);
	float bias = 0.00001f;
	float3 pos = input.positionLight.xyz / input.positionLight.w;
	float2 uv = float2(0, 0);
	uv.x = pos.x * 0.5f + 0.5f;
	uv.y = -pos.y * 0.5 + 0.5f;
	float depthInShadowMap = tex0.Sample(s0, uv).r;

	float d = pos.z - bias;

	if(d > depthInShadowMap)
		return ambient * input.color;

	return( input.color );
}

//-----------------------------------------------------------------------------
VS_OUTPUT VSShadowTargetMain( in VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrixLight);
	o.color = v.color;

	return o;
}


//-----------------------------------------------------------------------------
float4 PSShadowTargetMain( in VS_OUTPUT input ) : SV_Target
{
	return input.color;
}