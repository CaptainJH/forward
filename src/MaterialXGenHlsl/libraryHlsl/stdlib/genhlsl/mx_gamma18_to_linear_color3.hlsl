void mx_gamma18_to_linear_color3(float3 _in, out float3 result)
{
    result = pow(max((float3)(0.0), _in), (float3)(1.8));
}
