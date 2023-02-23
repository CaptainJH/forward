float2 mx_transform_uv(float2 uv, float2 uv_scale, float2 uv_offset)
{
    float2 uv2 = uv * uv_scale + uv_offset;
    return float2(uv2.x, 1.0 - uv2.y);
}
