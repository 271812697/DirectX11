#define PI       3.141592653589793
#define PI2      6.283185307179586
float3 UV2Cartesian(float2 st, uint face)
{
    float3 v = float3(0.0,0.0,0.0); // texture lookup vector in world space
    float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0; // convert [0, 1] to [-1, 1] and invert y

    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
    switch (face)
    {
        case 0:
            v = float3(+1.0, uv.y, -uv.x);
            break; // posx
        case 1:
            v = float3(-1.0, uv.y, uv.x);
            break; // negx
        case 2:
            v = float3(uv.x, +1.0, -uv.y);
            break; // posy
        case 3:
            v = float3(uv.x, -1.0, uv.y);
            break; // negy
        case 4:
            v = float3(uv.x, uv.y, +1.0);
            break; // posz
        case 5:
            v = float3(-uv.x, uv.y, -1.0);
            break; // negz
    }

    return normalize(v);
}

float3 ILS2Cartesian(uint3 w, float2 resolution)
{
    // w often comes from a compute shader in the form of `gl_GlobalInvocationID`
    float2 st = w.xy / resolution; // tex coordinates in [0, 1] range
    return UV2Cartesian(st, w.z);
}


float2 Spherical2Equirect(float2 v)
{
    return float2(v.x + 0.5, v.y); // ~ [0, 1]
}

float2 Cartesian2Spherical(float3 v)
{
    float phi = atan2(v.z, v.x); // ~ [-PI, PI] (assume v is normalized)
    float theta = acos(v.y); // ~ [0, PI]
    return float2(phi / PI2, theta / PI); // ~ [-0.5, 0.5], [0, 1]
}
Texture2D HDR : register(t20);
SamplerState g_SamLinear : register(s1);
RWTexture2DArray<float4> cubemap : register(u0);
[numthreads(32, 32, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    int w, h, d;
    cubemap.GetDimensions(w, h, d);
    
    float2 resolution = float2(w,h);
   
    float3 v = ILS2Cartesian(DTid, resolution);

    float2 sample_vec = Cartesian2Spherical(v);
    sample_vec = Spherical2Equirect(sample_vec);
    float4 color = HDR.SampleLevel(g_SamLinear, sample_vec,0);
    cubemap[DTid] = color;
}