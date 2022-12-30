struct VertexIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;

};
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 _uv : TEXCOORD0;
    float2 _uv1 : TEXCOORD1;
};
#ifdef VS

VertexOut main(VertexIn vIn) {
    VertexOut res;
    res.PosH=float4(vIn.pos,1.0);
    res._uv=vIn.uv0;
    res._uv1=vIn.uv1;  
    return res;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PS

SamplerState g_SamLinear : register(s0);
Texture2D color_texture : register(t0);
Texture2D bloom_texture:register(t1);


float3 ApproxACES(float3 radiance)
{
    float3 v = radiance * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
}
float3 Linear2Gamma(float3 color)
{
    return pow(color, 1.0 / 2.2); // component-wise
}
float4  main(VertexOut pIn):SV_TARGET {
    float3 color=float3(0.0,0.0,0.0);
    color+=color_texture.Sample(g_SamLinear, pIn._uv);
    color=ApproxACES(color);
    color+=bloom_texture.Sample(g_SamLinear, pIn._uv);
   
    return float4(Linear2Gamma(color),1.0);
}

#endif
