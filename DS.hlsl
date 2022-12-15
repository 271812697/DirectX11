struct DS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;     // 在世界中的位置
	float4 Color : COLOR;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;     // 在世界中的位置
	float4 Color : COLOR;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;
	float3 N = cross(patch[1].PosW - patch[0].PosW, patch[2].PosW - patch[0].PosW);
	Output.PosH = patch[0].PosH *domain.x+patch[1].PosH *domain.y+patch[2].PosH *domain.z;
	Output.PosW = patch[0].PosW * domain.x + patch[1].PosW * domain.y + patch[2].PosW * domain.z;
	//Output.PosH += float4(N* domain.x,0.0);
	Output.Color = patch[0].Color * domain.x + patch[1].Color * domain.y + patch[2].Color * domain.z;
	return Output;
}
