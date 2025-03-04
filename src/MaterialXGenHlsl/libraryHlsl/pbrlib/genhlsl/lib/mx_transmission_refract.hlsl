#include "mx_microfacet_specular.hlsl"

float3 mx_surface_transmission(float3 N, float3 V, float3 X, float2 alpha, int distribution, FresnelData fd)
{
    float3 transmission;
    if ($refractionEnv)
    {
        // Approximate the appearance of surface transmission as glossy
        // environment map refraction, ignoring any scene geometry that might
        // be visible through the surface.
        fd.refraction = true;
        transmission = mx_environment_radiance(N, V, X, alpha, distribution, fd);
    }
    else
    {
        transmission = $refractionColor;
    }

    return transmission;
}
