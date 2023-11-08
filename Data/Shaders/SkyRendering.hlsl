
cbuffer PerFrameCB : register(b0)
{
    float angle;
}

RWTexture2D<float4> outputBuffer        : register(u0);   

static int width = 1024;
static int height = 1024;

static float M_PI = 3.141592f;
static float kInfinity = 1.0e38f;

static float3 betaR = {3.8e-6f, 13.5e-6f, 33.1e-6f};
static float3 betaM = {21e-6f, 21e-6f, 21e-6f};

static float earthRadius = 6360e3f;      // In the paper this is usually Rg or Re (radius ground, eart)
static float atmosphereRadius = 6420e3f; // In the paper this is usually R or Ra (radius atmosphere)
static float Hr = 7994;               // Thickness of the atmosphere if density was uniform (Hr)
static float Hm = 1200;               // Same as above but for Mie scattering (Hm)

bool solveQuadratic(float a, float b, float c, out float x1, out float x2)
{
    if (b == 0) {
        // Handle special case where the the two vector ray.dir and V are perpendicular
        // with V = ray.orig - sphere.centre
        if (a == 0) return false;
        x1 = 0; x2 = sqrt(-c / a);
        return true;
    }
    float discr = b * b - 4 * a * c;

    if (discr < 0) return false;

    float q = (b < 0.f) ? -0.5f * (b - sqrt(discr)) : -0.5f * (b + sqrt(discr));
    x1 = q / a;
    x2 = c / q;

    return true;
}

// [comment]
// A simple routine to compute the intersection of a ray with a sphere
// [/comment]
bool raySphereIntersect(float3 orig, float3 dir, float radius, out float t0, out float t1)
{
    // They ray dir is normalized so A = 1 
    float A = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
    float C = orig.x * orig.x + orig.y * orig.y + orig.z * orig.z - radius * radius;

    if (!solveQuadratic(A, B, C, t0, t1)) return false;

    if (t0 > t1) //std::swap(t0, t1);
    {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    return true;
}

float3 ComputeIncidentLight(float3 orig, float3 dir, float tmin, float tmax)
{
    float t0, t1;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) 
        return 0.0f;
    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;

    float3 sunDirection = {sin(angle), cos(angle), 0};
    uint numSamples = 16;
    uint numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
    float3 sumR = 0;
    float3 sumM = 0; // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.f / (16.f * M_PI) * (1.f + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * M_PI) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));
    for (uint i = 0; i < numSamples; ++i) {
        float3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = length(samplePosition) - earthRadius;
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength;
        float hm = exp(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, t0Light, t1Light);
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        uint j = 0;
        for (j = 0; j < numSamplesLight; ++j) {
            float3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;
            float heightLight = length(samplePositionLight) - earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight) {
            float3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            float3 attenuation = {exp(-tau.x), exp(-tau.y), exp(-tau.z)};
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }

    // [comment]
    // We use a magic number here for the intensity of the sun (20). We will make it more
    // scientific in a future revision of this lesson/code
    // [/comment]
    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20;
}

[numthreads(8, 8, 1)]
void SkyMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 v = float4(0, 0, 0, 1);
    float y = 2.f * (DTid.y + 0.5f) / float(height - 1) - 1.f;
    float x = 2.f * (DTid.x + 0.5f) / float(width - 1) - 1.f;
    float z2 = x * x + y * y;
    if (z2 <= 1) 
    {
        float phi = atan2(y, x);
        float theta = acos(1 - z2);
        float3 dir = float3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
        // 1 meter above sea level
        float3 pos = float3(0, earthRadius + 1, 0);
        float3 skyColor = ComputeIncidentLight(pos, dir, 0, kInfinity);
        v = float4(skyColor, 1);
    }
    outputBuffer[DTid.xy] = v;
}