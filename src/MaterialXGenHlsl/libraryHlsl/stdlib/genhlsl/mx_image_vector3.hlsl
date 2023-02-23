#include "lib/$fileTransformUv"

void mx_image_vector3(sampler2D tex_sampler, int layer, float3 defaultval, float2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, float2 uv_scale, float2 uv_offset, out float3 result)
{
    if (textureSize(tex_sampler, 0).x > 1)
    {
        float2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
        result = texture(tex_sampler, uv).rgb;
    }
    else
    {
        result = defaultval;
    }
}
