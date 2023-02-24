#include "lib/mx_noise.hlsl"

void mx_noise3d_fa_vector2(float amplitude, float pivot, float3 position, out float2 result)
{
    float3 value = mx_perlin_noise_vec3(position);
    result = value.xy * amplitude + pivot;
}
