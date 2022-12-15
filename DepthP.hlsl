
cbuffer LIGHTPOS : register(b1)
{
    float3 lightPos;
}
struct Pixel
{
    float4 color : SV_TARGET;
    float depth : SV_Depth;
};
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 worldPos : POSITION;
};
Pixel main(VertexOut pIn) 
{
    Pixel ou;
    ou.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    ou.depth = distance(lightPos,pIn.worldPos)/1000.0f;
	return ou;
}