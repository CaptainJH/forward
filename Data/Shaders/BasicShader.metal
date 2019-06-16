#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct VertexIn 
{
    packed_float3 position;
    packed_float4 color;
};

struct VertexOut 
{
    float4 position [[position]];
    float4 color;
};

typedef struct __attribute__((__aligned__(64)))
{
    matrix_float4x4 WorldViewProjMatrix;
}transforms_t;

vertex VertexOut VSMainQuad(device const VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]]) 
{
    VertexOut out;
    out.position = float4(vertices[vertexId].position, 1);
    out.color = vertices[vertexId].color;
    return out;
}

vertex VertexOut VSMain_P_N_T_UV(device const VertexIn *vertices [[buffer(0)]], uint vertexId [[vertex_id]], constant transforms_t& transform [[buffer(1)]])
{
    VertexOut out;
    out.position = transform.WorldViewProjMatrix * float4(vertices[vertexId].position, 1);
    out.color = vertices[vertexId].color;
    return out;
}

fragment float4 PSMainQuad(VertexOut in [[stage_in]]) 
{
    return in.color;
}
