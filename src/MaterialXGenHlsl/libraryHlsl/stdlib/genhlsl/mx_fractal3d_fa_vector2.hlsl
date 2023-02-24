#include "lib/mx_noise.hlsl"

void mx_fractal3d_fa_vector2(float amplitude, int octaves, float lacunarity, float diminish, float3 position, out float2 result)
{
    float2 value = mx_fractal_noise_vec2(position, octaves, lacunarity, diminish);
    result = value * amplitude;
}
