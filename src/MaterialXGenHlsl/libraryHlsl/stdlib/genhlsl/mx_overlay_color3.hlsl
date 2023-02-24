#include "mx_overlay.hlsl"

void mx_overlay_color3(float3 fg, float3 bg, float mix, out float3 result)
{
    result = mix * mx_overlay(fg, bg) + (1.0-mix) * bg;
}
