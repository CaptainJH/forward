#include "lib/mx_transform_color.hlsl"

void mx_ap1_to_rec709_color3(float3 _in, out float3 result)
{
    result = M_AP1_TO_REC709 * _in;
}
