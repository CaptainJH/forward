#include "mx_overlay.hlsl"

void mx_overlay_color4(float4 fg, float4 bg, float mix, out float4 result)
{
    result = mix * mx_overlay(fg, bg) + (1.0-mix) * bg;
}
