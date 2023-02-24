#include "lib/mx_noise.hlsl"

void mx_noise2d_fa_vector3(float amplitude, float pivot, float2 texcoord, out float3 result)
{
    float3 value = mx_perlin_noise_vec3(texcoord);
    result = value * amplitude + pivot;
}
