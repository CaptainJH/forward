#include "lib/mx_noise.hlsl"

void mx_noise2d_fa_vector2(float amplitude, float pivot, float2 texcoord, out float2 result)
{
    float3 value = mx_perlin_noise_vec3(texcoord);
    result = value.xy * amplitude + pivot;
}
