// 物体表面材质
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};


struct VertexIn {
    float3 PosL : POSITION;
    float3 Normal : NORMAL;
    float4 tangent : TANGENT;
    float2 tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 Normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
}

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    float4 w =mul(float4(vIn.PosL, 1.0), g_World);
    vOut.PosW = w.xyz;
    vOut.PosH = mul(w, g_ViewProj);
    vOut.Normal = mul(vIn.Normal, (float3x3) g_WorldInvTranspose);
    vOut.texcoord = vIn.tex;
   
    return vOut;
}
