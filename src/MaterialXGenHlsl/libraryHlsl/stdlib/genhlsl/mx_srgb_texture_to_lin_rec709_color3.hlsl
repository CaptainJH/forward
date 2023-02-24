#include "lib/mx_transform_color.hlsl"

void mx_srgb_texture_to_lin_rec709_color3(float3 _in, out float3 result)
{
    result = mx_srgb_texture_to_lin_rec709(_in);
}
