#include "lib/$fileTransformUv"

void mx_image_float(Texture2D tex_sampler, int layer, float defaultval, float2 texcoord, int uaddressmode, int vaddressmode, int filtertype, int framerange, int frameoffset, int frameendaction, float2 uv_scale, float2 uv_offset, out float result)
{
    int w, h;
    tex_sampler.GetDimensions(w, h);
    if (w > 1)
    {
        float2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
        result = tex_sampler.Sample(s, uv).r;
    }
    else
    {
        result = defaultval;
    }
}
