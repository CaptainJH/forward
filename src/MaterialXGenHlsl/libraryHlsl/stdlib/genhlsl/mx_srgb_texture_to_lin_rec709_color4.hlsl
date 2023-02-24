#include "lib/mx_transform_color.hlsl"

void mx_srgb_texture_to_lin_rec709_color4(float4 _in, out float4 result)
{
    result = float4(mx_srgb_texture_to_lin_rec709(_in.rgb), _in.a);
}
