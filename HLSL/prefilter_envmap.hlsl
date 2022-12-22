#define PI       3.141592653589793
#define PI2      6.283185307179586
#define INV_PI   0.318309886183791  // 1 over PI

TextureCube environment_map : register(t0);
SamplerState g_SamLinear : register(s0);
RWTexture2DArray<float4> prefilter_map : register(u0);
cbuffer ConstantBuffer : register(b0)
{
    float roughness=0.1;
}

// the Van der Corput radical inverse sequence
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // (1 / 0x100000000)
}
// the Hammersley point set (a low-discrepancy random sequence)
float2 Hammersley2D(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}


float3 Tangent2World(float3 N, float3 v)
{
    N = normalize(N);

    // choose the up vector U that does not overlap with N
    
    float3 U;
    if (step(abs(N.y), 0.999))
        U = float3(0.0, 1.0, 0.0);
    else
        U = float3(1.0, 0.0, 0.0);
    //mix(float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), step(abs(N.y), 0.999));
    //float3 U = float3(0.0, 1.0, 0.0);
    float3 T = normalize(cross(U, N));
    float3 B = normalize(cross(N, T));
    return T * v.x + B * v.y + N * v.z; // mat3(T, B, N) * v
}
// Trowbridge-Reitz GGX normal distribution function (long tail distribution)
// Bruce Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
float D_TRGGX(float alpha, float NoH)
{
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}
// importance sampling with GGX NDF for a given alpha (roughness squared)
// this also operates on the unit hemisphere, and the PDF is D_TRGGX() * cosine
// this function returns the halfway vector H (because NDF is evaluated at H)
float3 ImportanceSampleGGX(float u, float v, float alpha)
{
    float a2 = alpha * alpha;
    float phi = u * PI2;
    float cos_theta = sqrt((1.0 - v) / (1.0 + (a2 - 1.0) * v)); // bias toward cosine and TRGGX NDF
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

float3 PrefilterEnvironmentMap(float3 R, uint n_samples)
{
    float3 N = R;
    float3 V = R;
    int we = 0, he = 0,pad;
    environment_map.GetDimensions(0,we,he,pad);
    float2 env_size = float2(we,he);
    float w = 4.0 * PI / (6 * env_size.x * env_size.y); // solid angle per texel (Equation 12)

    // roughness is guaranteed to be > 0 as the base level is copied directly
    float alpha = roughness * roughness;

    float weight = 0.0;
    float3 color = float3(0.0,0.0,0.0);

    for (uint i = 0; i < n_samples; i++)
    {
        float2 u = Hammersley2D(i, n_samples);
        float3 H = Tangent2World(N, ImportanceSampleGGX(u.x, u.y, alpha));
        float3 L = 2 * dot(H, V) * H - V;

        float NoH = max(dot(N, H), 0.0);
        float NoL = max(dot(N, L), 0.0);
        float HoV = max(dot(H, V), 0.0);

        if (NoL > 0.0)
        {
            float pdf = D_TRGGX(NoH, alpha) * NoH / (4.0 * HoV);
            float ws = 1.0 / (n_samples * pdf + 0.0001); // solid angle associated with this sample (Equation 11)
            float mip_level = max(0.5 * log2(ws / w) + 1.0, 0.0); // mipmap level (Equation 13 biased by +1)
            
            color += environment_map.SampleLevel(g_SamLinear, L, mip_level).rgb * NoL;
            weight += NoL;
        }
    }

    return color / weight;
}
// convert a 2D texture coordinate st on a cubemap face to its equivalent 3D
// texture lookup vector v such that `texture(cubemap, v) == texture(face, st)`
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
// convert an ILS image coordinate w to its equivalent 3D texture lookup
// vector v such that `texture(samplerCube, v) == imageLoad(imageCube, w)`
float3 ILS2Cartesian(uint3 w, float2 resolution)
{
    // w often comes from a compute shader in the form of `gl_GlobalInvocationID`
    float2 st = w.xy / resolution; // tex coordinates in [0, 1] range
    return UV2Cartesian(st, w.z);
}
[numthreads(32,32,1)]
void CS(uint3 DTid:SV_DispatchThreadID)
{
 
    uint3 ils_coordinate = uint3(DTid);
    int w, h, d;
    prefilter_map.GetDimensions(w,h,d);
    float2 resolution = float2(w,h);

    // make sure we won't write past the texture when computing higher mipmap levels
    if (ils_coordinate.x >= resolution.x || ils_coordinate.y >= resolution.y)
    {
        return;
    }

    float3 R = ILS2Cartesian(ils_coordinate, resolution);
    float3 color = PrefilterEnvironmentMap(R, 2048); // ~ 1024 samples for 2K resolution
    prefilter_map[DTid] = float4(color, 1.0);
}