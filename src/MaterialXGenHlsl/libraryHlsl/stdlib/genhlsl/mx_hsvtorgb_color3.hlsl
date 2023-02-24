#include "lib/mx_hsv.hlsl"

void mx_hsvtorgb_color3(float3 _in, out float3 result)
{
    result = mx_hsvtorgb(_in);
}
