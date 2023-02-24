void mx_gamma24_to_linear_color4(float4 _in, out float4 result)
{
    float4 gamma = float4(2.4, 2.4, 2.4, 1.0);
    result = pow(max((float4)(0.0), _in), gamma);
}
