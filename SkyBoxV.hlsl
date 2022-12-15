struct VertexIn
{
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // �������е�λ��
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