
#ifdef VS
struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
};
cbuffer VSConstantBuffer : register(b3)
{
    matrix World;
    matrix View;
    matrix Proj;
}
VertexOut main(VertexIn vIn) 
{
    VertexOut vOut;
    vOut.PosH = float4(vIn.PosL, 0);
    vOut.PosH = mul(vOut.PosH, View);
    vOut.PosH.w = 1;
    vOut.PosH = mul(vOut.PosH, Proj);
    vOut.PosH = vOut.PosH.xyww;
    vOut.PosW = vIn.PosL;
    return vOut;
}
#endif
#ifdef PS
struct Vertex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
};
TextureCube g_TexCube : register(t26);
SamplerState g_SamLinear : register(s1);
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
float4 main(Vertex v) : SV_TARGET
{
    
    float3 color = g_TexCube.SampleLevel(g_SamLinear, v.PosW.xyz,0).rgb;
    return float4(color, 0.5);
}
#endif