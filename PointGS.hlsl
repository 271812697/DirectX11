struct Vertex
{
	float4 PosH : SV_POSITION;//裁剪的齐次坐标
	float3 PosW : POSITION;     // 在世界中的位置
	float4 Color : COLOR;
};
struct GSOutput
{
	float4 PosH : SV_POSITION;
};
cbuffer GSConstantBuffer : register(b6)
{
	matrix WVP;
	matrix WVPInv;
}
cbuffer GSSphereConstantBuffer : register(b9)
{
	float3 spherepos[50];
	uint sphereindex[252];
}

[maxvertexcount(256)]
void main(
	point Vertex input[1],
	inout TriangleStream< GSOutput > output
)
{
	GSOutput element;
	element.PosH = input[0].PosH;
    
	for (int i = 0; i < 84; i++) {
        element.PosH = mul(float4(input[0].PosW + spherepos[sphereindex[3 * i]], 1.0), WVP);
		output.Append(element);
        element.PosH = mul(float4(input[0].PosW + spherepos[sphereindex[3 * i + 1]], 1.0), WVP);
		output.Append(element);
        element.PosH = mul(float4(input[0].PosW + spherepos[sphereindex[3 * i + 2]], 1.0), WVP);
		output.Append(element);
		output.RestartStrip();

	}
}