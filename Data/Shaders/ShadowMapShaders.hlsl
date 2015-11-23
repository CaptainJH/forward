//-----------------------------------------------------------------------------
cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	matrix WorldViewProjMatrixLight;
	float4 flags;
	matrix CSMViewProjMatrix[3];
};

Texture2D<float> tex0: register( t0 );
SamplerState s0: register( s0 );
SamplerComparisonState PCFSampler : register( s1 );

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

struct GS_OUTPUT
{
	float4 Pos		: SV_POSITION;
	float4 Color 	: COLOR;
	float4 positionLight : PositionLightSpace;	
	uint RTIndex	: SV_RenderTargetArrayIndex;
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
	float4 ambient = float4(0.01, 0.01, 0.01, 1);
	float bias = 0.0001f;
	float3 pos = input.positionLight.xyz / input.positionLight.w;
	float2 uv = float2(0, 0);
	uv.x = pos.x * 0.5f + 0.5f;
	uv.y = -pos.y * 0.5 + 0.5f;
	float depthInShadowMap = tex0.Sample(s0, uv).r;
	float depthInShadowMapPCF = tex0.SampleCmpLevelZero(PCFSampler, uv, pos.z - bias).r;

	float d = pos.z - bias;

	bool usePCF = (flags.x >= 1.0f);

	if(usePCF)
	{
		return input.color * depthInShadowMapPCF;
	}
	else
	{
		if(d >= depthInShadowMap)
			return ambient * input.color;

		return( input.color );
	}
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

//-----------------------------------------------------------------------------
VS_OUTPUT VSCSMTargetMain(in VS_INPUT v)
{
	VS_OUTPUT o = (VS_OUTPUT)0;
	o.position = v.position;
	o.color = v.color;

	return o;
}

///////////////////////////////////////////////////////////////////
// Cascaded shadow maps generation
///////////////////////////////////////////////////////////////////
[maxvertexcount(9)]
void GSCSMTargetMain(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
	for(int iFace = 0; iFace < 3; iFace++ )
	{
		GS_OUTPUT output;

		output.RTIndex = iFace;

		for(int v = 0; v < 3; v++ )
		{
			output.Pos = mul(InPos[v], CSMViewProjMatrix[iFace]);
			output.Color = float4(1, 0, 0, 1);
			output.positionLight = output.Pos;
			OutStream.Append(output);
		}
		OutStream.RestartStrip();
	}
}