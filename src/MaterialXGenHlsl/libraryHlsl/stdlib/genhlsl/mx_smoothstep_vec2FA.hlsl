#include "mx_smoothstep_float.hlsl"

void mx_smoothstep_vec2FA(float2 val, float low, float high, out float2 result)
{
    mx_smoothstep_float(val.x, low, high, result.x);
    mx_smoothstep_float(val.y, low, high, result.y);
}
