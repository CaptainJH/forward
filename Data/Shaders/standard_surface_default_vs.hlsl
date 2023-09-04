// Generated by JHQ using MaterialXGenHlsl 

// Uniform block: PrivateUniforms
cbuffer PrivateUniforms_vertex : register(b0)
{
    float4x4 u_worldMatrix;
    float4x4 u_viewProjectionMatrix;
    float4x4 u_worldInverseTransposeMatrix;
};

// Inputs block: VertexInputs
struct VertexInputs
{
    float3 i_position : POSITION;
    float3 i_normal : NORMAL;
    float3 i_tangent : TANGENT;
};

struct VertexData
{
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float3 positionWorld : POSITION;
    float4 position : SV_POSITION;
};

VertexData main( in VertexInputs i_vs )
{
    VertexData vd = (VertexData)0;
    float4 hPositionWorld = mul(float4(i_vs.i_position, 1.0), u_worldMatrix);
    vd.position = mul(hPositionWorld, u_viewProjectionMatrix);
    vd.normalWorld = normalize(mul(float4(i_vs.i_normal, 0.0), u_worldInverseTransposeMatrix)).xyz;
    vd.tangentWorld = normalize(mul(float4(i_vs.i_tangent, 0.0),u_worldMatrix)).xyz;
    vd.positionWorld = hPositionWorld.xyz;
    return vd;
}
