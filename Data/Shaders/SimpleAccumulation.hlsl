
cbuffer PerFrameCB : register(b0)
{
    uint gAccumCount;
}

RWTexture2D<float4> outputBuffer        : register(u0);   
RWTexture2D<float4> accumulationBuffer  : register(u1);

[numthreads(8, 8, 1)]
void AccumulationMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 v = outputBuffer[DTid.xy];
    if(gAccumCount > 0)
        accumulationBuffer[DTid.xy] = accumulationBuffer[DTid.xy] + v;
    else
        accumulationBuffer[DTid.xy] = v;
    outputBuffer[DTid.xy] = accumulationBuffer[DTid.xy] / (gAccumCount + 1);
}

Texture2D<float4> lastFrameBuffer     : register(t0);

// A work-around function because AccumulationMain may cause TDR error in some cases
[numthreads(8, 8, 1)]
void AccumulationMain_WAR(uint3 DTid : SV_DispatchThreadID)
{
    float4 v = lastFrameBuffer[DTid.xy];
    if(gAccumCount > 0)
        accumulationBuffer[DTid.xy] = accumulationBuffer[DTid.xy] + v;
    else
        accumulationBuffer[DTid.xy] = v;
    outputBuffer[DTid.xy] = accumulationBuffer[DTid.xy] / (gAccumCount + 1);
}