#include "lib/mx_noise.hlsl"

void mx_cellnoise2d_float(vec2 texcoord, out float result)
{
    result = mx_cell_noise_float(texcoord);
}
