//-----------------------------------------------------------------------------
cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	matrix WorldViewProjMatrixLight;
	float4 flags;
	matrix CSMViewProjMatrix[3];
	float4 toCascadeOffsetX;
	float4 toCascadeOffsetY;
	float4 toCascadeScale;
};

Texture2D<float> tex0: register( t0 );
Texture2DArray<float> tex1 : register( t1 );
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
	float bias = 0.000001f;
	float3 pos = input.positionLight.xyz / input.positionLight.w;
	float2 uv = float2(0, 0);
	uv.x = pos.x * 0.5f + 0.5f;
	uv.y = -pos.y * 0.5f + 0.5f;
	float d = pos.z - bias;	
	float depthInShadowMap = tex0.Sample(s0, uv).r;
	float depthInShadowMapPCF = tex0.SampleCmpLevelZero(PCFSampler, uv, d).r;

	bool usePCF = (flags.x >= 1.0f);
	bool useCSM = (flags.y >= 1.0f);

	if(useCSM)
	{
		// Transform the shadow space position into each cascade position
		float4 posCascadeSpaceX = (toCascadeOffsetX + pos.xxxx) * toCascadeScale;
		float4 posCascadeSpaceY = (toCascadeOffsetY + pos.yyyy) * toCascadeScale;

		// Check which cascade we are in
		float4 inCascadeX = abs(posCascadeSpaceX) <= 1.0;
		float4 inCascadeY = abs(posCascadeSpaceY) <= 1.0;
		float4 inCascade = inCascadeX * inCascadeY;

		// Prepare a mask for the highest quality cascade the position is in
		float4 bestCascadeMask = inCascade;
		bestCascadeMask.yzw = (1.0 - bestCascadeMask.x) * bestCascadeMask.yzw;
		bestCascadeMask.zw = (1.0 - bestCascadeMask.y) * bestCascadeMask.zw;
		bestCascadeMask.w = (1.0 - bestCascadeMask.z) * bestCascadeMask.w;
		float bestCascade = dot(bestCascadeMask, float4(0.0, 1.0, 2.0, 3.0));

		// Pick the position in the selected cascade
		float3 UVD;
		UVD.x = dot(posCascadeSpaceX, bestCascadeMask);
		UVD.y = dot(posCascadeSpaceY, bestCascadeMask);
		UVD.z = pos.z;

		// Convert to shadow map UV values
		UVD.xy = 0.5 * UVD.xy + 0.5;
		UVD.y = 1.0 - UVD.y;

		depthInShadowMap = tex1.Sample(s0, float3(UVD.xy, bestCascade));
		depthInShadowMapPCF = tex1.SampleCmpLevelZero(PCFSampler, float3(UVD.xy, bestCascade), d);

		if(usePCF)
			return input.color * depthInShadowMapPCF;
		else
		{
			if(d >= depthInShadowMap)
				return ambient * input.color;
			return input.color;
		}
	}
	else if(usePCF)
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