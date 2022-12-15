
// 方向光
struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

// 点光
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

// 聚光灯
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

// 物体表面材质
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
    // 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 光向量与照射方向相反
    float3 lightVec = -L.direction;

    // 添加环境光
    ambient = mat.ambient * L.ambient;

    // 添加漫反射光和镜面光
    float diffuseFactor = dot(lightVec, normal);

    // 展开，避免动态分支
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
    // 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 从表面到光源的向量
    float3 lightVec = L.position - pos;

    // 表面到光线的距离
    float d = length(lightVec);

    // 灯光范围测试
    if (d > L.range)
        return;

    // 标准化光向量
    lightVec /= d;

    // 环境光计算
    ambient = mat.ambient * L.ambient;

    // 漫反射和镜面计算
    float diffuseFactor = dot(lightVec, normal);

    // 展开以避免动态分支
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // 光的衰弱
    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}


void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
    out float4 ambient, out float4 diffuse, out float4 spec)
{
    // 初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // // 从表面到光源的向量
    float3 lightVec = L.position - pos;

    // 表面到光源的距离
    float d = length(lightVec);

    // 范围测试
    if (d > L.range)
        return;

    // 标准化光向量
    lightVec /= d;

    // 计算环境光部分
    ambient = mat.ambient * L.ambient;


    // 计算漫反射光和镜面反射光部分
    float diffuseFactor = dot(lightVec, normal);

    // 展开以避免动态分支
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // 计算汇聚因子和衰弱系数
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
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float2 tex : TEXCOORD;
};

Texture2D g_DiffuseMap : register(t0);
TextureCube depthtexture_Cube : register(t5);
SamplerState g_Sam : register(s0);
// 像素着色器(3D)
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
    // 提前进行Alpha裁剪，对不符合要求的像素可以避免后续运算
    //clip(texColor.a - 0.1f);
    
    // 标准化法向量
        pIn.normalW = normalize(pIn.normalW);

    // 顶点指向眼睛的向量
    float3 toEyeW = normalize(g_EyePosW - pIn.posW);

    // 初始化为0 
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
