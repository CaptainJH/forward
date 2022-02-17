//-----------------------------------------------------------------------------
cbuffer Transforms : register(b0)
{
	matrix WorldViewProjMatrix;	
};

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct VS_INPUT_P_UV
{
	float4 position : POSITION;
	float2 uv		: TEXCOORD0;
};

struct VS_INPUT_P_N_T_UV
{
	float4 position : POSITION;
	float4 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain( in VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = v.color;

	return o;
}

VS_OUTPUT VSMain_P_UV(in VS_INPUT_P_UV v)
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = float4(0.0f, 0, 0, 0);
	o.uv = v.uv;

	return o;
}

VS_OUTPUT VSMain_P_N_T_UV(in VS_INPUT_P_N_T_UV v)
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = float4(1.0f, 0, 0, 0);
	o.uv = v.uv;

	return o;
}

Texture2D baseTexture : register(t0);
TextureCube cubeTexture : register(t1);
SamplerState baseSampler : register(s0);
//-----------------------------------------------------------------------------
float4 PSMain( in VS_OUTPUT input ) : SV_Target
{
	float4 color = baseTexture.Sample(baseSampler, input.uv);
	return color;
	//return input.color;
}


VS_OUTPUT VSMainQuad( in VS_INPUT v, uint vid : SV_VertexID )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = v.position;
	o.color = v.color;

	return o;
}
float4 PSMainQuad( in VS_OUTPUT input ) : SV_Target
{
	return input.color;
}