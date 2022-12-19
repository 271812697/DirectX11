
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
#ifdef VS
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


#endif
#ifdef PS
float4 main(VertexOut pIn) : SV_Target
{ 
    return float4(1.0, 1.0, 1.0, 1.0);

}
#endif

