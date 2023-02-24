#include "lib/mx_noise.hlsl"

void mx_noise3d_fa_vector4(float amplitude, float pivot, float3 position, out float4 result)
{
    float3 xyz = mx_perlin_noise_vec3(position);
    float w = mx_perlin_noise_float(position + float3(19, 73, 29));
    result = float4(xyz, w) * amplitude + pivot;
}
