
#ifdef VS
cbuffer WVP: register(b0)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;

}
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
    float3 _position : POSITION; // 在世界中的位置
    float3 _normal : NORMAL;
    float3 _tangent : TANGENT;
    float3 _binormal : BINORMAL;
    float2 _uv : TEXCOORD0;
    float2 _uv2 : TEXCOORD1;
};


VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    float4 w =mul(float4(vIn.pos, 1.0), g_World);
    //float4 w =float4(vIn.PosL, 1.0);
    vOut._position = w.xyz;
    vOut.PosH = mul(w, g_View);
    vOut.PosH = mul(vOut.PosH,g_Proj);
    vOut._normal = normalize(mul(vIn.normal, (float3x3) g_World));
    vOut._tangent = normalize(mul(vIn.tangent, (float3x3)g_World));
    vOut._binormal = normalize(cross(vOut._normal, vOut._tangent));
    vOut._uv = vIn.uv0;
    vOut._uv2 = vIn.uv1;
    return vOut;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PS
struct PixelIn
{
    float4 PosH : SV_POSITION;
    float3 _position : POSITION; // 在世界中的位置
    float3 _normal : NORMAL;
    float3 _tangent : TANGENT;
    float3 _binormal : BINORMAL;
    float2 _uv : TEXCOORD0;
    float2 _uv2 : TEXCOORD1;
};
struct PixelOut
{
    float4 color : SV_TARGET0;
    float4 bloom : SV_TARGET1;
};

cbuffer BLOOM : register(b1)
{
    float4 light_color;
    float light_intensity;
    float bloom_factor;
    float time;
}




PixelOut main() {
    PixelOut res;
    float fade_io = 0.3 + abs(cos(time));
    float intensity = light_intensity * fade_io;

    // if the 2nd MRT isn't enabled, bloom will write to GL_NONE and be discarded
    res.color = float4(light_color.rgb * intensity, 1.0);
    res.bloom = float4(res.color.rgb * bloom_factor, 1.0);
    //bloom=vec4(color.rgb * bloom_factor, 1.0);
    return res;

}

#endif
