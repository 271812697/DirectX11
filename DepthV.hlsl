cbuffer CBChangesEveryFrame : register(b0)
{
    matrix g_World;
    matrix g_ViewProj;
}
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 worldPos : POSITION;
};
VertexOut main(float3 pos : POSITION) 
{
    VertexOut vres;
    float4 res = mul(float4(pos,1.0f), g_World);
    vres.worldPos = res.xyz / res.w;
    res = mul(res,g_ViewProj);
    vres.PosH = res;
	return vres;
}