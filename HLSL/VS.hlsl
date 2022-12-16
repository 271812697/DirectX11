/*
                static const D3D11_INPUT_ELEMENT_DESC inputLayout[8] = {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD0", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD1", 0,DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TANGENT", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 } ,
                    { "BINORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "BONEID", 0,DXGI_FORMAT_R32G32B32A32_SINT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "BONEWE", 0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };
*/

struct VertexIn {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    int4 bone_id : BONEID;
    float4 bone_wt : BONEWE;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix W;
    matrix V;
    matrix P;
}
VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    float4 w =mul(float4(vIn.pos, 1.0), W);
    vOut.PosW = w.xyz;
    vOut.PosH = mul(w, V);
    vOut.PosH = mul(vOut.PosH, P);
    vOut.normal = mul(vIn.normal, (float3x3) W);
    vOut.tangent = mul(vIn.tangent, (float3x3) W);
    vOut.binormal = mul(vIn.binormal, (float3x3) W);
    vOut.uv0 = vIn.uv0;
    vOut.uv1 = vIn.uv1;
    return vOut;
}
