#include "mx_microfacet_specular.hlsl"

// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
// Section 20.4 Equation 13
float mx_latlong_compute_lod(float3 dir, float pdf, float maxMipLevel, int envSamples)
{
    const float MIP_LEVEL_OFFSET = 1.5;
    float effectiveMaxMipLevel = maxMipLevel - MIP_LEVEL_OFFSET;
    float distortion = sqrt(1.0 - mx_square(dir.y));
    return max(effectiveMaxMipLevel - 0.5 * log2(float(envSamples) * pdf * distortion), 0.0);
}

float3 mx_environment_radiance(float3 N, float3 V, float3 X, float2 alpha, int distribution, FresnelData fd)
{
    // Generate tangent frame.
    float3 Y = normalize(cross(N, X));
    X = cross(Y, N);
    float3x3 tangentToWorld = float3x3(X, Y, N);

    // Transform the view vector to tangent space.
    V = float3(dot(V, X), dot(V, Y), dot(V, N));

    // Compute derived properties.
    float NdotV = clamp(V.z, M_FLOAT_EPS, 1.0);
    float avgAlpha = mx_average_alpha(alpha);
    
    // Integrate outgoing radiance using filtered importance sampling.
    // http://cgg.mff.cuni.cz/~jaroslav/papers/2008-egsr-fis/2008-egsr-fis-final-embedded.pdf
    float3 radiance = (float3)0.0;
    int envRadianceSamples = $envRadianceSamples;
    for (int i = 0; i < envRadianceSamples; i++)
    {
        float2 Xi = mx_spherical_fibonacci(i, envRadianceSamples);

        // Compute the half vector and incoming light direction.
        float3 H = mx_ggx_importance_sample_NDF(Xi, alpha);
        float3 L = fd.refraction ? mx_refraction_solid_sphere(-V, H, fd.ior.x) : -reflect(V, H);
        
        // Compute dot products for this sample.
        float NdotH = clamp(H.z, M_FLOAT_EPS, 1.0);
        float NdotL = clamp(L.z, M_FLOAT_EPS, 1.0);
        float VdotH = clamp(dot(V, H), M_FLOAT_EPS, 1.0);
        float LdotH = VdotH;

        // Sample the environment light from the given direction.
        float3 Lw = mul(L, tangentToWorld);
        float pdf = mx_ggx_PDF(H, LdotH, alpha);
        float lod = mx_latlong_compute_lod(Lw, pdf, float($envRadianceMips - 1), envRadianceSamples);
        float3 sampleColor = mx_latlong_map_lookup(Lw, $envMatrix, lod, $envRadiance);

        // Compute the Fresnel term.
        float3 F = mx_compute_fresnel(VdotH, fd);

        // Compute the geometric term.
        float G = mx_ggx_smith_G2(NdotL, NdotV, avgAlpha);

        // Compute the combined FG term, which is inverted for refraction.
        float3 FG = fd.refraction ? (float3)(1.0) - (F * G) : F * G;

        // Add the radiance contribution of this sample.
        // From https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
        //   incidentLight = sampleColor * NdotL
        //   microfacetSpecular = D * F * G / (4 * NdotL * NdotV)
        //   pdf = D * NdotH / (4 * VdotH)
        //   radiance = incidentLight * microfacetSpecular / pdf
        radiance += sampleColor * FG * VdotH / (NdotV * NdotH);
    }

    // Normalize and return the final radiance.
    radiance /= float(envRadianceSamples);
    return radiance;
}

float3 mx_environment_irradiance(float3 N)
{
    return mx_latlong_map_lookup(N, $envMatrix, 0.0, $envIrradiance);
}
