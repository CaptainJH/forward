//-----------------------------------------------------------------------------
cbuffer SDFParams
{
	float4 screenParams;
	float4 camPos;
	float4 SDFParams;
	float4 SDFOrigin;
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
	float2 uv : TEXCOORD;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMainQuad( in VS_INPUT v, uint vid : SV_VertexID )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	o.position = v.position;
	if(vid == 0)
	{
		o.color.xyz = float3(0, 0, 0);
		o.uv = float2(0, 1);
	}
	else if(vid == 1)
	{
		o.color.xyz = float3(1, 0, 0);
		o.uv = float2(1, 1);
	}
	else if(vid == 2)
	{
		o.color.xyz = float3(0, 1, 1);
		o.uv = float2(0, 0);
	}
	else if(vid == 3)
	{
		o.color.xyz = float3(1, 1, 1);
		o.uv = float2(1, 0);
	}

	return o;
}

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

float3 checkPixel(float2 screenPos, float4 posScn)
{
	float width = screenParams.x;
	float height = screenParams.y;

	float x = 2.0f * screenPos.x / width - 1;
	float y = -2.0f * screenPos.y / height + 1;

	return float3(abs(x - posScn.x), abs(y - posScn.y), 0.0f);	
}

void GetVolumePos(out float3 minPos, out float3 maxPos)
{
	minPos = SDFOrigin.xyz;
	maxPos = minPos + SDFParams.xyz * SDFParams.w;
}

bool isInsideVolume(float3 pos)
{
	float3 minPos; 
	float3 maxPos;
	GetVolumePos(minPos, maxPos);

	if(pos.x < minPos.x || pos.x > maxPos.x)
		return false;

	if(pos.y < minPos.y || pos.y > maxPos.y)
		return false;

	if(pos.z < minPos.z || pos.z > maxPos.z)
		return false;

	return true;
}

float3 Pos2VolumeUVW(float3 pos)
{
	float3 minPos; 
	float3 maxPos;
	GetVolumePos(minPos, maxPos);
	float dx = SDFParams.w;

	float3 N = (pos - minPos) / dx;
	return N / SDFParams.xyz;
}

float SampleVolume(float3 uvw)
{
	return tex0.Sample(s0, uvw);
}

float3 CalculateVolumeIntersectionPos(float3 rayOrigin, float3 rayDir)
{
	float3 minPos; 
	float3 maxPos;
	GetVolumePos(minPos, maxPos);

	float3 kNoIntersection = float3(-1000.0f, -1000.0f, -1000.0f);

	float xt;
	if(rayOrigin.x < minPos.x)
	{
		xt = minPos.x - rayOrigin.x;
		if(rayDir.x < 0.0f)
			return kNoIntersection;

		xt /= rayDir.x;
	}
	else if(rayOrigin.x > maxPos.x)
	{
		xt = maxPos.x - rayOrigin.x;
		if(rayDir.x > 0.0f)
			return kNoIntersection;

		xt /= rayDir.x;
	}
	else
	{
		xt = -1.0f;
	}

	float yt;
	if(rayOrigin.y < minPos.y)
	{
		yt = minPos.y - rayOrigin.y;
		if(rayDir.y < 0.0f)
			return kNoIntersection;

		yt /= rayDir.y;
	}
	else if(rayOrigin.y > maxPos.y)
	{
		yt = maxPos.y - rayOrigin.y;
		if(rayDir.y > 0.0f)
			return kNoIntersection;

		yt /= rayDir.y;
	}
	else
	{
		yt = -1.0f;
	}

	float zt;
	if(rayOrigin.z < minPos.z)
	{
		zt = minPos.z - rayOrigin.z;
		if(rayDir.z < 0.0f)
			return kNoIntersection;

		zt /= rayDir.z;
	}
	else if(rayOrigin.z > maxPos.z)
	{
		zt = maxPos.z - rayOrigin.z;
		if(rayDir.z > 0.0f)
			return kNoIntersection;

		zt /= rayDir.z;
	}
	else
	{
		zt = -1.0f;
	}

	int which = 0;
	float t = xt;
	if(yt > t)
	{
		which = 1;
		t = yt;
	}
	if(zt > t)
	{
		which = 2;
		t = zt;
	}

	if(which == 0)
	{
		float y = rayOrigin.y + rayDir.y * t;
		if(y < minPos.y || y > maxPos.y)
			return kNoIntersection;
		float z = rayOrigin.z + rayDir.z * t;
		if(z < minPos.z || z > maxPos.z)
			return kNoIntersection;
	}
	else if(which == 1)
	{
		float x = rayOrigin.x + rayDir.x * t;
		if(x < minPos.x || x > maxPos.x)
			return kNoIntersection;
		float z = rayOrigin.z + rayDir.z * t;
		if(z < minPos.z || z > maxPos.z)
			return kNoIntersection;
	}
	else if(which == 2)
	{
		float x = rayOrigin.x + rayDir.x * t;
		if(x < minPos.x || x > maxPos.x)
			return kNoIntersection;
		float y = rayOrigin.y + rayDir.y * t;
		if(y < minPos.y || y > maxPos.y)
			return kNoIntersection;					
	}


	return rayOrigin + rayDir * t;
}

float CalculateRayMarchingSteps(float3 origin, float3 dir)
{
	const int MaxSteps = 50;
	const float epsilon = 0.01f;

	bool inside = isInsideVolume(origin);
	float3 start = origin;
	float steps = 0;
	bool intersected = false;
	if(inside == false)
	{
		start = CalculateVolumeIntersectionPos(origin, dir);
		steps += 1;
	}

	if(start.x < -900.0f && start.y < -900.0f && start.z < -900.0f)
		return 0.0f;

	float df = 0.0f;

	for(int s = 0; s < MaxSteps; ++s)
	{
		float3 uvw = Pos2VolumeUVW(start);
		df = SampleVolume(uvw);

		if(df < epsilon)
		{
			intersected = true;
			break;
		}

		start += dir * df;
		steps += 1;

		if(isInsideVolume(start) == false)
		{
			break;
		}

	}

	if(intersected)
		return steps;
	return 0;
}

float3 PSMainQuad( in VS_OUTPUT input ) : SV_Target
{
	float4 pos = ConvertFromScreenCoordToWorldSpaceCoord(input.position.xy);
	//float4 posScn = mul(pos, ViewProjMatrix);
	//posScn = posScn / posScn.w;

	float3 dir = normalize(pos - camPos.xyz);
	float steps = CalculateRayMarchingSteps(camPos.xyz, dir);
	float value = steps / 400;

	return float3(value, value, value);
}