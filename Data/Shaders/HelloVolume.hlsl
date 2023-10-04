
RWTexture2D<float4> OutputTexture : register(u0);

static const float3 background_color = float3(0.572f, 0.772f, 0.921f);

cbuffer UniformData : register(b0)
{
    uint Size;
    float SphereRadius;
    float3 SphereCenter;
};

struct IsectData
{
    float t0;
    float t1;
    float3 pHit;
    float3 nHit;
    bool inside;
};

bool solveQuadratic(float a, float b, float c, out float r0, out float r1)
{
    float d = b * b - 4 * a * c;
    if (d < 0) return false;
    else if (d == 0) r0 = r1 = -0.5f * b / a;
    else {
        float q = (b > 0) ? -0.5f * (b + sqrt(d)) : -0.5f * (b - sqrt(d));
        r0 = q / a;
        r1 = c / q;
    }

    if (r0 > r1) 
    {
        float t = r0;
        r0 = r1;
        r1 = t;
    }

    return true;
}

bool IntersectSphere(float3 rayOrigin, float3 rayDir, out IsectData isect)
{
    float3 rayOrigc = rayOrigin - float3(0, 0, -20);
    float a = dot(rayDir, rayDir);
    float b = dot(rayDir, rayOrigc) * 2;
    float c = dot(rayOrigc, rayOrigc) - SphereRadius * SphereRadius;

    if (!solveQuadratic(a, b, c, isect.t0, isect.t1)) return false;

    if (isect.t0 < 0) {
        if (isect.t1 < 0) return false;
        else {
            isect.inside = true;
            isect.t0 = 0;
        }
    }

    return true;
}

float3 Integrate(float3 rayDir)
{
    float3 result = 0.0f;
    float3 rayOrigin = float3(0, 0, 0);

    IsectData isect;
    if(IntersectSphere(rayOrigin, rayDir, isect))
    {
        float step_size = 0.2f;
        float absorption = 0.1f;
        float scattering = 0.1f;
        float density = 1.0f;
        int ns = ceil((isect.t1 - isect.t0) / step_size);
        step_size = (isect.t1 - isect.t0) / ns;

        float3 light_dir = float3(0, 1, 0);
        float3 light_color = float3(1.3f, 0.3f, 0.9f);
        IsectData isect_vol;

        float transparency = 1; // initialize transmission to 1 (fully transparent)

        // [comment]
        // The ray-marching loop (forward, march from t0 to t1)
        // [/comment]
        for (int n = 0; n < ns; ++n) 
        {
            float t = isect.t0 + step_size * (n + 0.5f);
            float3 sample_pos = rayOrigin + t * rayDir;

            // compute sample transmission
            float sample_attenuation = exp(-step_size * (scattering + absorption));
            transparency *= sample_attenuation;

            // In-scattering. Find distance light travels through volumetric sphere to the sample.
            // Then use Beer's law to attenuate the light contribution due to in-scattering.
            if (IntersectSphere(sample_pos, light_dir, isect_vol) && isect_vol.inside) 
            {
                float light_attenuation = exp(-density * isect_vol.t1 * (scattering + absorption));
                result += transparency * light_color * light_attenuation * scattering * density * step_size;
            }
        }

        result += background_color * transparency;
    }
    else
        result = background_color;

    return result;
}


[numthreads(8, 8, 1)]
void DrawVolume(uint3 DTid : SV_DispatchThreadID)
{
    float focal = tan(radians(45.0f * 0.5f));
	float i = (float(DTid.x) + 0.5) / float(Size);
	float j = (float(DTid.y) + 0.5) / float(Size);
    float3 rayDir = float3(
        (2.0f * i - 1) * focal,
        (1.0f - 2.0f * j) * focal, -1.0f);
    rayDir = normalize(rayDir);

	OutputTexture[DTid.xy] = float4(Integrate(rayDir), 1.0f);
}