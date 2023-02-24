#include "lib/mx_noise.hlsl"

void mx_cellnoise3d_float(float3 position, out float result)
{
    result = mx_cell_noise_float(position);
}
