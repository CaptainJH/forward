#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct VS_INPUT 
{
    float2 modelPosition [[attribute(0)]];
    float2 modelTCoord   [[attribute(1)]];
};

struct VS_OUTPUT 
{
    float4 clipPosition [[position]];
    float2 vertexTCoord;
};

struct Translate
{
    float4 translate;
};

struct TextColor
{
    float4 textColor;
};

vertex VS_OUTPUT VSMain(const VS_INPUT input [[stage_in]], constant Translate& transform [[buffer(1)]]) 
{
    VS_OUTPUT output;
    output.vertexTCoord = input.modelTCoord;

    float2 modelPos = input.modelPosition;
    output.clipPosition.x = 2.0f*modelPos.x - 1.0f + 2.0f*transform.translate.x;
    output.clipPosition.y = 2.0f*modelPos.y - 1.0f + 2.0f*transform.translate.y;
    output.clipPosition.z = 0.0f;
    output.clipPosition.w = 1.0f;

    return output;
}

fragment float4 PSMain(VS_OUTPUT input [[stage_in]], texture2d<half> colorTexture [[texture(0)]], constant TextColor& textColor [[buffer(0)]]) 
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);

    // Sample the texture to obtain a color
    const float bitmapAlpha = colorTexture.sample(textureSampler, input.vertexTCoord).r;

    // return the color of the texture
    return textColor.textColor * (1.0f - bitmapAlpha);
}
