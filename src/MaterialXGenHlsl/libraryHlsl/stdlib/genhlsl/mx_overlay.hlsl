float mx_overlay(float fg, float bg)
{
    return (fg < 0.5) ? (2.0 * fg * bg) : (1.0 - (1.0 - fg) * (1.0 - bg));
}

float2 mx_overlay(float2 fg, float2 bg)
{
    return float2(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g));
}

float3 mx_overlay(float3 fg, float3 bg)
{
    return float3(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g),
                mx_overlay(fg.b, bg.b));
}

float4 mx_overlay(float4 fg, float4 bg)
{
    return float4(mx_overlay(fg.r, bg.r),
                mx_overlay(fg.g, bg.g),
                mx_overlay(fg.b, bg.b),
                mx_overlay(fg.a, bg.a));
}
