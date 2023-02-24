#include "mx_smoothstep_float.hlsl"

void mx_smoothstep_vec3FA(float3 val, float low, float high, out float3 result)
{
    mx_smoothstep_float(val.x, low, high, result.x);
    mx_smoothstep_float(val.y, low, high, result.y);
    mx_smoothstep_float(val.z, low, high, result.z);
}
