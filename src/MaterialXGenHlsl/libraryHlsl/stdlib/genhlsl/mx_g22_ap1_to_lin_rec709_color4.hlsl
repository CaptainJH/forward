#include "lib/mx_transform_color.hlsl"

void mx_g22_ap1_to_lin_rec709_color4(float4 _in, out float4 result)
{
    result = float4(M_AP1_TO_REC709 * pow(max((float3)(0.0), _in.rgb), (float3)(2.2)), _in.a);
}
