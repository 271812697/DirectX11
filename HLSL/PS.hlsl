struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

float4 main(VertexOut pIn) : SV_Target
{ 
    return float4(1.0, 1.0, 1.0, 1.0);

}
