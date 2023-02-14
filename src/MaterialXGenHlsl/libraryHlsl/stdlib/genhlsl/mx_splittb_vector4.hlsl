#include "mx_aastep.hlsl"

void mx_splittb_vector4(float4 valuet, float4 valueb, float center, float2 texcoord, out float4 result)
{
    result = lerp(valuet, valueb, mx_aastep(center, texcoord.y));
}
