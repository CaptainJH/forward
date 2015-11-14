//-----------------------------------------------------------------------------
cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	float4 Distance;
};

Texture2D tex0: register( t0 );;
SamplerState s0: register( s0 );;

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
};

struct GS_INPUTOUTPUT
{
	float4 position			: SV_Position;
	float4 color			: COLOR;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain( in VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = mul(v.position, WorldViewProjMatrix);
	o.color = v.color;

	return o;
}


//-----------------------------------------------------------------------------
float4 PSMain( in GS_INPUTOUTPUT input ) : SV_Target
{
	float threshold = 0.8f;
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float d = min(input.color.x, min(input.color.y, input.color.z));
	if(d < threshold && Distance.w > 0)
		color = Distance;

	return( color );
}


VS_OUTPUT VSMainQuad( in VS_INPUT v, uint vid : SV_VertexID )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = v.position;
	o.color = v.color;

	return o;
}
float4 PSMainQuad( in GS_INPUTOUTPUT input ) : SV_Target
{
	return input.color;
}