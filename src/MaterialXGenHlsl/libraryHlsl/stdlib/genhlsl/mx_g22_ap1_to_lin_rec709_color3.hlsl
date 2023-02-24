#include "lib/mx_transform_color.hlsl"

void mx_g22_ap1_to_lin_rec709_color3(float3 _in, out float3 result)
{
    result = M_AP1_TO_REC709 * pow(max((float3)(0.0), _in), (float3)(2.2));
}
