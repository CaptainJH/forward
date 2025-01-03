#include "mx_microfacet_specular.hlsl"

float mx_latlong_compute_lod(float alpha)
{
    // Select a mip level based on input alpha.
    float lodBias = alpha < 0.25 ? sqrt(alpha) : 0.5*alpha + 0.375;
    return lodBias * float($envRadianceMips);
}

float3 mx_environment_radiance(float3 N, float3 V, float3 X, float2 alpha, int distribution, FresnelData fd)
{
    N = mx_forward_facing_normal(N, V);
    float3 L = fd.refraction ? mx_refraction_solid_sphere(-V, N, fd.ior.x) : -reflect(V, N);

    float NdotV = clamp(dot(N, V), M_FLOAT_EPS, 1.0);

    float avgAlpha = mx_average_alpha(alpha);
    float3 F = mx_compute_fresnel(NdotV, fd);
    float G = mx_ggx_smith_G2(NdotV, NdotV, avgAlpha);
    float3 FG = fd.refraction ? (float3)(1.0) - (F * G) : F * G;

    float3 Li = mx_latlong_map_lookup(L, $envMatrix, mx_latlong_compute_lod(avgAlpha), $envRadiance);
    return Li * FG;
}

float3 mx_environment_irradiance(float3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance);
}
