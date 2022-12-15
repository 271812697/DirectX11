struct VertexIn
{
    float3 PosL : POSITION;
    float3 Normal : NORMAL;
    float4 tangent : TANGENT;
    float2 tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;

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
    vOut.PosH = mul(float4(vIn.PosL, 1.0),World);
    vOut.PosH = mul(vOut.PosH, View);
    vOut.PosH = mul(vOut.PosH, Proj);
    return vOut;
}
