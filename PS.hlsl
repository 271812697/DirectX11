
// �����
struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

// ���
struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 att;
    float pad;
};

// �۹��
struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 direction;
    float Spot;

    float3 att;
    float pad;
};

// ����������
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};



void ComputeDirectionalLight(Material mat, DirectionalLight L,
    float3 normal, float3 toEye,
    out float4 ambient,
    out float4 diffuse,
    out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �����������䷽���෴
    float3 lightVec = -L.direction;

    // ��ӻ�����
    ambient = mat.ambient * L.ambient;

    // ����������;����
    float diffuseFactor = dot(lightVec, normal);

    // չ�������⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
}


void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // �ӱ��浽��Դ������
    float3 lightVec = L.position - pos;

    // ���浽���ߵľ���
    float d = length(lightVec);

    // �ƹⷶΧ����
    if (d > L.range)
        return;

    // ��׼��������
    lightVec /= d;

    // ���������
    ambient = mat.ambient * L.ambient;

    // ������;������
    float diffuseFactor = dot(lightVec, normal);

    // չ���Ա��⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // ���˥��
    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}


void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // ��ʼ�����
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // // �ӱ��浽��Դ������
    float3 lightVec = L.position - pos;

    // ���浽��Դ�ľ���
    float d = length(lightVec);

    // ��Χ����
    if (d > L.range)
        return;

    // ��׼��������
    lightVec /= d;

    // ���㻷���ⲿ��
    ambient = mat.ambient * L.ambient;


    // �����������;��淴��ⲿ��
    float diffuseFactor = dot(lightVec, normal);

    // չ���Ա��⶯̬��֧
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // ���������Ӻ�˥��ϵ��
    float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.Spot);
    float att = spot / dot(L.att, float3(1.0f, d, d * d));

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
    int haveT;
}

cbuffer CBChangesRarely : register(b2)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}
cbuffer Light : register(b4)
{
    float3 lightPos;
}

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // �������е�λ��
    float3 normalW : NORMAL; // �������������еķ���
    float2 tex : TEXCOORD;
};

Texture2D g_DiffuseMap : register(t0);
TextureCube depthtexture_Cube : register(t5);
SamplerState g_Sam : register(s0);
// ������ɫ��(3D)
float LinearizeDepth(float depth, float near, float far)
{
    float z = depth; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}
float4 main(VertexPosHWNormalTex pIn) : SV_Target
{ 
    float pcfDepth = depthtexture_Cube.Sample(g_Sam, pIn.posW - lightPos).x * 1000;
    float dis = distance(pIn.posW, lightPos);
    if (dis > pcfDepth +0.01)
        return float4(1.0,1.0,0.0,1.0);
    
    float4 texColor = g_DiffuseMap.Sample(g_Sam, pIn.tex);
    if (haveT==0)texColor = float4(0.2,0.2,0.2,1.0);
    // ��ǰ����Alpha�ü����Բ�����Ҫ������ؿ��Ա����������
    //clip(texColor.a - 0.1f);
    
    // ��׼��������
        pIn.normalW = normalize(pIn.normalW);

    // ����ָ���۾�������
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);

    // ��ʼ��Ϊ0 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;

    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
        
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputePointLight(g_Material, g_PointLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    [unroll]
    for (i = 0; i < 5; ++i)
    {
        ComputeSpotLight(g_Material, g_SpotLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
  
    
    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.diffuse.a;
    return litColor;
}
