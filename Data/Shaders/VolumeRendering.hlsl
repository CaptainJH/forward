
cbuffer PerFrameCB : register(b0)
{
    float4x4 ProjectionToWorld;
    float4   CameraPosition;
}

RWTexture2D<float4> outputBuffer        : register(u0);  
// StructuredBuffer<float> Grid            : register(t0);

static int width = 640;
static int height = 480;

static float M_PI = 3.141592f;
static float kInfinity = 1.0e38f;

static float3 backgroundColor = { 0.572f, 0.772f, 0.921f };

static float3 bound_min = { -30.0f, -30.0f, -30.0f };
static float3 bound_max = { 30.0f, 30.0f, 30.0f };

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

void trace(Ray ray, out float3 L, out float transmittance)
{
    float tmin, tmax;
    if (raybox(ray, tmin, tmax))
    {
        //integrate(ray, tmin, tmax, L, transmittance, grid);
        L = float3(1, 0, 0);
        transmittance = 0.0f;
    }
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

    float3 L = 0.0f; // radiance for that ray (light collected)
    float transmittance = 1;
    trace(ray, L, transmittance);
    v.rgb += backgroundColor * transmittance + L;

    outputBuffer[DTid.xy] = v;
}