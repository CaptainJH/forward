#include "hlslUtils.hlsl"

cbuffer PerFrameCB : register(b0)
{
    float4x4 ProjectionToWorld;
    float4   CameraPosition;
    int      FrameCount;
    int      width;
    int      height;
    int      VolumeIndex;
}

RWTexture2D<float4> outputBuffer        : register(u0);  
StructuredBuffer<float> Grid[60]        : register(t0);

static float M_PI = 3.141592f;
static float kInfinity = 1.0e38f;

static float3 backgroundColor = { 0.572f, 0.772f, 0.921f };

static float3 bound_min = { -30.0f, -30.0f, -30.0f };
static float3 bound_max = { 30.0f, 30.0f, 30.0f };

static uint baseResolution = 128;

struct Ray
{
    float3 rayOrigin;
    float3 rayDir;
};

float max_component(float4 v)
{
    float2 maxPairs = max(v.xy, v.zw);
    return max(maxPairs.x, maxPairs.y);
}

float min_component(float4 v)
{
    float2 minPairs = min(v.xy, v.zw);
    return min(minPairs.x, minPairs.y);
}

float phaseHG(float3 viewDir, float3 lightDir, float g)
{
    float costheta = dot(viewDir, lightDir);
    return 1 / (4 * M_PI) * (1 - g * g) / pow(1 + g * g - 2 * g * costheta, 1.5f);
}

// the ray AABB intersection test comes from Ray Tracing Gems 2
// Chapter2: RAY AXIS-ALIGNED BOUNDING BOX INTERSECTION
bool raybox(Ray ray, out float tmin, out float tmax)
{
    float3 invRaydir = { 1 / ray.rayDir.x, 1 / ray.rayDir.y, 1 / ray.rayDir.z };
    float3 rayOrigin = ray.rayOrigin;
    float3 tLower = (bound_min - rayOrigin) * invRaydir;
    float3 tUpper = (bound_max - rayOrigin) * invRaydir;
    // The four t-intervals (for x-/y-/z-slabs, and ray p(t))
    float4 tMins = { min(tLower, tUpper), 0.0f };
    float4 tMaxes = { max(tLower, tUpper), 10000000.0f };
    // Easy to remember: ``max of mins, and min of maxes''
    float tBoxMin = max_component(tMins);
    float tBoxMax = min_component(tMaxes);
    tmin = tBoxMin;
    tmax = tBoxMax;

    return tBoxMin <= tBoxMax;
}

float grid(int xi, int yi, int zi)
{
    if (xi < 0 || xi > baseResolution - 1 ||
        yi < 0 || yi > baseResolution - 1 ||
        zi < 0 || zi > baseResolution - 1)
        return 0;

    return Grid[VolumeIndex][(zi * baseResolution + yi) * baseResolution + xi];
}

float lookup(float3 p)
{
    const float3 gridSize = bound_max - bound_min;
    const float3 pLocal = (p - bound_min) / gridSize;
    const float3 pVoxel = pLocal * baseResolution;

    float3 pLattice = {pVoxel.x - 0.5f, pVoxel.y - 0.5f, pVoxel.z - 0.5f};
    int xi = floor(pLattice.x);
    int yi = floor(pLattice.y);
    int zi = floor(pLattice.z);

    // trilinear filtering
    float weight[3];
    float value = 0;

    for (int i = 0; i < 2; ++i) 
    {
        weight[0] = 1 - abs(pLattice.x - (xi + i));
        for (int j = 0; j < 2; ++j) 
        {
            weight[1] = 1 - abs(pLattice.y - (yi + j));
            for (int k = 0; k < 2; ++k) 
            {
                weight[2] = 1 - abs(pLattice.z - (zi + k));
                value += weight[0] * weight[1] * weight[2] * grid(xi + i, yi + j, zi + k);
            }
        }
    }

    return value;
}

void integrate(
    Ray ray,                        // camera ray 
    float tMin, float tMax,         // range of integration
    out float3 L,                   // radiance (out)
    out float T,
    uint randSeed)                    // transmission (out)
{
    const float stepSize = 0.05f;
    float sigma_a = 0.5f;
    float sigma_s = 0.5f;
    float sigma_t = sigma_a + sigma_s;
    float g = 0; // henyey-greenstein asymetry factor 
    uint d = 2; // russian roulette "probability" 
    float shadowOpacity = 1;

    uint numSteps = ceil((tMax - tMin) / stepSize);
    float stride = (tMax - tMin) / numSteps;

    float3 lightDir = {-0.315798f, 0.719361f, 0.618702f};
    float3 lightColor = 20;

    float3 Lvol = 0.0f;
    float Tvol = 1;

    for (uint n = 0; n < numSteps; ++n) 
    {
        float t = tMin + stride * (n + 0.5f);
        float3 samplePos = ray.rayOrigin + ray.rayDir * t;

        //[comment]
        // Read density from the 3D grid
        //[/comment]
        float density = lookup(samplePos);

        float Tsample = exp(-stride * density * sigma_t);
        Tvol *= Tsample;

        float tlMin, tlMax;
        Ray lightRay = {samplePos, samplePos + lightDir};
        if (density > 0 && raybox(lightRay, tlMin, tlMax) && tlMax > 0) 
        {
            uint numStepsLight = ceil(tlMax / stepSize);
            float strideLight = tlMax / numStepsLight;
            float densityLight = 0;
            for (uint nl = 0; nl < numStepsLight; ++nl) 
            {
                float tLight = strideLight * (nl + 0.5f);
                //[comment]
                // Read density from the 3D grid
                //[/comment]
                densityLight += lookup(lightRay.rayOrigin + lightRay.rayDir * tLight);
            }
            float lightRayAtt = exp(-densityLight * strideLight * sigma_t * shadowOpacity);
            Lvol += lightColor * lightRayAtt * phaseHG(-ray.rayDir, lightDir, g) * sigma_s * Tvol * stride * density;
        }

        if (Tvol < 1e-3) 
        {
            if (nextRand(randSeed) > 1.f / d)
                break;
            else
                Tvol *= d;
        }
    }

    L = Lvol;
    T = Tvol;
}

void trace(Ray ray, out float3 L, out float transmittance, uint randSeed)
{
    float tmin, tmax;
    if (raybox(ray, tmin, tmax))
        integrate(ray, tmin, tmax, L, transmittance, randSeed);
}

[numthreads(8, 8, 1)]
void VolumeMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 v = float4(0, 0, 0, 1);
    float x = 2.f * (DTid.x + 0.5f) / width - 1.f;
    float y = 2.f * (DTid.y + 0.5f) / height - 1.f;
    float2 screenPos = float2(x, -y);

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), ProjectionToWorld);

    world.xyz /= world.w;
    float3 rayOrigin = CameraPosition.xyz;
    float3 rayDirection = normalize(world.xyz - rayOrigin);
    Ray ray = { rayOrigin, rayDirection };

    // Initialize our random number generator
    uint randSeed = initRand(DTid.x + DTid.y * width, FrameCount, 16);

    float3 L = 0.0f; // radiance for that ray (light collected)
    float transmittance = 1;
    trace(ray, L, transmittance, randSeed);
    v.rgb += (backgroundColor * transmittance + L) * 0.8f;

    outputBuffer[DTid.xy] = v;
}