
struct Vertex
{
	float4 PosH : SV_POSITION;//�ü����������
	float3 PosW : POSITION;     // �������е�λ��
	float4 Color : COLOR;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;//�ü����������
	float3 PosW : POSITION;     // �������е�λ��
	float3 Nor :NORMAL;
	float4 Color : COLOR;
};

[maxvertexcount(3)]
void GS(
	triangle Vertex input[3],
	inout TriangleStream< GSOutput > output
)
{
	float3 N = cross(input[1].PosW-input[0].PosW, input[2].PosW - input[0].PosW);
	N = normalize(N);
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.PosH = input[i].PosH;
		element.PosW = input[i].PosW;
		element.Color = input[i].Color;
		element.Nor = N;
		output.Append(element);
	}
}