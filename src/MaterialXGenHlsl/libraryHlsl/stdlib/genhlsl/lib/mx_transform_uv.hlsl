float2 mx_transform_uv(float2 uv, float2 uv_scale, float2 uv_offset)
{
    float2 uv2 = uv * uv_scale + uv_offset;
    return uv2;
}
