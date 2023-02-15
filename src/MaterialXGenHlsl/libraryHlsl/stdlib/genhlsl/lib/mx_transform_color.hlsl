#define M_AP1_TO_REC709 float3x3(1.705079555511475, -0.1297005265951157, -0.02416634373366833, -0.6242334842681885, 1.138468623161316, -0.1246141716837883, -0.0808461606502533, -0.008768022060394287, 1.148780584335327)

float3 mx_srgb_texture_to_lin_rec709(float3 color)
{
    bvec3 isAbove = greaterThan(color, (float3)(0.04045));
    float3 linSeg = color / 12.92;
    float3 powSeg = pow(max(color + (float3)(0.055), (float3)(0.0)) / 1.055, (float3)(2.4));
    return lerp(linSeg, powSeg, isAbove);
}
