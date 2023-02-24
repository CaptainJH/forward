#include "lib/mx_transform_color.hlsl"

void mx_ap1_to_rec709_color4(float4 _in, out float4 result)
{
    result = float4(M_AP1_TO_REC709 * _in.rgb, _in.a);
}
