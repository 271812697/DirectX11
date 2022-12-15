struct VS_CONTROL_POINT_OUTPUT
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = 1;

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.PosH = ip[i].PosH;
	Output.PosW = ip[i].PosW;
	Output.Color = ip[i].Color;
	return Output;
}
