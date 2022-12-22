#define PI2      6.283185307179586
TextureCube g_TexCube : register(t0);
SamplerState g_SamLinear : register(s0);
RWTexture2DArray<float4> irradiance_map : register(u0);
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
    else U=float3(1.0, 0.0, 0.0);
    //mix(float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), step(abs(N.y), 0.999));
    //float3 U = float3(0.0, 1.0, 0.0);
	float3 T = normalize(cross(U, N));
	float3 B = normalize(cross(N, T));
	return T * v.x + B * v.y + N * v.z; // mat3(T, B, N) * v
}
// cosine-weighted point sampling on the unit hemisphere
// the probability of a point being sampled is (cosine / PI), i.e. biased by cosine
// this method is favored over uniform sampling for cosine-weighted rendering equations
float3 CosineSampleHemisphere(float u, float v)
{
	float phi = v * PI2;
	float cos_theta = sqrt(1.0 - u); // bias toward cosine using the `sqrt` function
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
	return float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}
// this way of sampling over the hemisphere is cosine-weighted, preciser and faster.
// for 2K resolution we only need ~ 8000 samples to achieve nice results.
float3 CosineConvolution(float3 N, uint n_samples)
{
	float3 irradiance = float3(0.0,0.0,0.0);

	for (uint i = 0; i < n_samples; i++)
	{
		float2 u = Hammersley2D(i, n_samples);
		float3 L = CosineSampleHemisphere(u.x, u.y);
		L = Tangent2World(N, L);
        
        irradiance += g_TexCube.SampleLevel(g_SamLinear, L,0).rgb;
       
    }

    /* since the sampling is already cosine-weighted, we can directly sum up the retrieved texel
       values and divide by the total number of samples, there's no need to include a weight and
       then balance the result with a multiplier. If we multiply each texel by `NoL` and then
       double the result as we did in uniform sampling, we are essentially weighing the radiance
       twice, in which case the result irradiance map would be less blurred where bright pixels
       appear brighter and dark areas are darker, in fact many people were doing this wrong.
    */

	return irradiance / float(n_samples);
}

// convert a 2D texture coordinate st on a cubemap face to its equivalent 3D
// texture lookup vector v such that `texture(cubemap, v) == texture(face, st)`
float3 UV2Cartesian(float2 st, uint face)
{
    float3 v = float3(0.0, 0.0, 0.0); // texture lookup vector in world space
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
    int w, h, d;
    irradiance_map.GetDimensions(w,h,d);
	uint3 ils_coordinate = uint3(DTid);
	float2 resolution = float2(w,h);
	float3 N = ILS2Cartesian(ils_coordinate, resolution);

    // here we present 3 different ways of computing diffuse irradiance map from an HDR
    // environment map, all 3 have considered the cosine term in the integral, and will
    // yield results that are hardly distinguishable. The last one uses cosine-weighted
    // sampling, it's a lot more performant and requires much fewer samples to converge.

    // vec3 irradiance = NaiveConvolution(N, 0.01, 0.01);
    // vec3 irradiance = UniformConvolution(N, 16384);
	float3 irradiance = CosineConvolution(N, 16384);

    irradiance_map[DTid] = float4(irradiance,1.0);

}
