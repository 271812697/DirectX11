struct VertexIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;

};
struct VertexOut {
    float4 PosH : SV_POSITION;
    float2 _uv : TEXCOORD0;
    float2 _uv1 : TEXCOORD1;
};
#ifdef VS

VertexOut main(VertexIn vIn) {
    VertexOut res;
    res.PosH=float4(vIn.pos,1.0);
    res._uv=vIn.uv0;
    res._uv1=vIn.uv1;  
    return res;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef PS

SamplerState g_SamLinear : register(s0);
Texture2D color_texture : register(t0);
Texture2D depth_texture:register(t1);
const float near = 0.1;
const float far = 100.0;
uint index=0;


float4  main(VertexOut pIn):SV_TARGET {
    float4 color=float4(1.0,0.0,0.0,1.0);

    if(index==0){
        color = color_texture.Sample(g_SamLinear, pIn._uv);
    
        
        
     }
    
    else{
     
            float depth = depth_texture.Sample(g_SamLinear,pIn._uv).r;  // ~ [0,1] but non-linear
  // depth = depth * 2.0 - 1.0;  // ~ [-1,1]
    float z = (2.0 * near * far) / (far + near - depth * (far - near));  // ~ [near, far]
    float linear_depth = z / far;  // ~ [0,1]
    color =float4(linear_depth, linear_depth,linear_depth,1.0);
        


     }
    return color;
}

#endif
