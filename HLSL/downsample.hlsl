
Texture2D input : register(t0);
RWTexture2D<float4> output : register(u0);
SamplerState g_SamLinear : register(s0);

[numthreads(32, 18, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int w, h;
    output.GetDimensions(w,h);
    float2 uv = float2(float(DTid.x) / float(w), float(DTid.y) / float(h));
    output[DTid.xy] = input.SampleLevel(g_SamLinear, uv,0);
}



