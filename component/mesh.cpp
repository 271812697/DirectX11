#include "mesh.h"
#include"../util/math.h"
#include"../util/global.h"
namespace component {
	using namespace util;

	//ComPtr<ID3D11InputLayout> Mesh::m_pVertexLayout;
	void component::Mesh::CreateSphere(float radius)
	{
		constexpr float PI = 3.141592654;
		constexpr float PI_2 = 3.141592654 / 2;

		// default LOD = 100x100 mesh grid size
		unsigned int n_rows = 100;
		unsigned int n_cols = 100;
		unsigned int n_verts = (n_rows + 1) * (n_cols + 1);
		unsigned int n_tris = n_rows * n_cols * 2;

		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		vertices.reserve(n_verts);
		indices.reserve(n_tris * 3);
		for (unsigned int col = 0; col <= n_cols; ++col) {
			for (unsigned int row = 0; row <= n_rows; ++row) {
				// unscaled uv coordinates ~ [0, 1]
				float u = static_cast<float>(col) / n_cols;
				float v = static_cast<float>(row) / n_rows;

				float theta = PI * v - PI_2;  // ~ [-PI/2, PI/2], latitude from south to north pole
				float phi = PI * 2 * u;       // ~ [0, 2PI], longitude around the equator circle

				float x = cos(phi) * cos(theta);
				float y = sin(theta);
				float z = sin(phi) * cos(theta) ;

				// for a unit sphere centered at the origin, normal = position
				// binormal is normal rotated by 90 degrees along the latitude (+theta)
				theta += PI_2;
				float r = cos(phi) * cos(theta);
				float s = sin(theta);
				float t = sin(phi) * cos(theta) ;
				Vertex vertex{};
				vertex.position = DirectX::XMFLOAT3{ x * radius, y * radius, z * radius };
				vertex.normal = DirectX::XMFLOAT3{ x , y , z  };
				vertex.uv = DirectX::XMFLOAT2(u, v);
				vertex.binormal = DirectX::XMFLOAT3(r, s, t);
				vertex.tangent = util::Cross(vertex.binormal, vertex.normal);
				vertices.push_back(vertex);
			}
		}
		for (unsigned int col = 0; col < n_cols; ++col) {
			for (unsigned int row = 0; row < n_rows; ++row) {
				auto index = col * (n_rows + 1);

				// clockwise winding order
				indices.push_back(index + row + 1 + n_rows);		
				indices.push_back(index + row);
                indices.push_back(index + row + 1);
				// counter winding order
				indices.push_back(index + row + 1 + n_rows);
				indices.push_back(index + row + 1);
				indices.push_back(index + row + 1 + n_rows + 1);
			}
		}

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreateCube(float size)
	{
		using namespace DirectX;
		constexpr int n_vertices = 24;  // we only need 24 vertices to triangulate the 6 faces
	

		std::vector<Vertex> vertices;
		vertices.resize(n_vertices);
		float w2 = size / 2, h2 = size / 2, d2 = size / 2;

		// 右面(+X面)
		vertices[0].position = XMFLOAT3(w2, -h2, -d2);
		vertices[1].position = XMFLOAT3(w2, h2, -d2);
		vertices[2].position = XMFLOAT3(w2, h2, d2);
		vertices[3].position = XMFLOAT3(w2, -h2, d2);
		// 左面(-X面)
		vertices[4].position = XMFLOAT3(-w2, -h2, d2);
		vertices[5].position = XMFLOAT3(-w2, h2, d2);
		vertices[6].position = XMFLOAT3(-w2, h2, -d2);
		vertices[7].position = XMFLOAT3(-w2, -h2, -d2);
		// 顶面(+Y面)
		vertices[8].position = XMFLOAT3(-w2, h2, -d2);
		vertices[9].position = XMFLOAT3(-w2, h2, d2);
		vertices[10].position = XMFLOAT3(w2, h2, d2);
		vertices[11].position = XMFLOAT3(w2, h2, -d2);
		// 底面(-Y面)
		vertices[12].position = XMFLOAT3(w2, -h2, -d2);
		vertices[13].position = XMFLOAT3(w2, -h2, d2);
		vertices[14].position = XMFLOAT3(-w2, -h2, d2);
		vertices[15].position = XMFLOAT3(-w2, -h2, -d2);
		// 背面(+Z面)
		vertices[16].position = XMFLOAT3(w2, -h2, d2);
		vertices[17].position = XMFLOAT3(w2, h2, d2);
		vertices[18].position = XMFLOAT3(-w2, h2, d2);
		vertices[19].position = XMFLOAT3(-w2, -h2, d2);
		// 正面(-Z面)
		vertices[20].position = XMFLOAT3(-w2, -h2, -d2);
		vertices[21].position = XMFLOAT3(-w2, h2, -d2);
		vertices[22].position = XMFLOAT3(w2, h2, -d2);
		vertices[23].position = XMFLOAT3(w2, -h2, -d2);

		for (size_t i = 0; i < 4; ++i)
		{
			// 右面(+X面)
			vertices[i].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
			vertices[i].tangent = XMFLOAT3(0.0f, 0.0f, 1.0f);
			// 左面(-X面)
			vertices[i+4].normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			vertices[i+4].tangent = XMFLOAT3(0.0f, 0.0f, -1.0f);
			// 顶面(+Y面)
			vertices[i + 8].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertices[i + 8].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
			// 底面(-Y面)
			vertices[i + 12].normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
			vertices[i + 12].tangent = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			// 背面(+Z面)
			vertices[i + 16].normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
			vertices[i + 16].tangent = XMFLOAT3(-1.0f, 0.0f, 0.0f);
			// 正面(-Z面)
			vertices[i + 20].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
			vertices[i + 20].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
		}

		for (size_t i = 0; i < 6; ++i)
		{
			vertices[i * 4].uv = XMFLOAT2(0.0f, 1.0f);
			vertices[i * 4 + 1] .uv= XMFLOAT2(0.0f, 0.0f);
			vertices[i * 4 + 2].uv = XMFLOAT2(1.0f, 0.0f);
			vertices[i * 4 + 3].uv = XMFLOAT2(1.0f, 1.0f);
		}

		std::vector<UINT> indices{
			0, 1, 2, 2, 3, 0,		// 右面(+X面)
			4, 5, 6, 6, 7, 4,		// 左面(-X面)
			8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
			12, 13, 14, 14, 15, 12,	// 底面(-Y面)
			16, 17, 18, 18, 19, 16, // 背面(+Z面)
			20, 21, 22, 22, 23, 20	// 正面(-Z面)
		};

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreatePlane(float size)
	{
		using namespace DirectX;

		std::vector<Vertex> vertices;
		vertices.resize(4);
		float  width = size;
		float depth = size;
		uint32_t vIndex = 0;
		vertices[vIndex].position= XMFLOAT3(-width / 2, 0.0f, -depth / 2);
		vertices[vIndex].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertices[vIndex].tangent= XMFLOAT3(1.0f, 0.0f, 0.0f);
		vertices[vIndex++].uv= XMFLOAT2(0.0f, 1.0);

		vertices[vIndex].position = XMFLOAT3(-width / 2, 0.0f, depth / 2);
		vertices[vIndex].normal= XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertices[vIndex].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
		vertices[vIndex++].uv= XMFLOAT2(0.0f, 0.0f);

		vertices[vIndex].position = XMFLOAT3(width / 2, 0.0f, depth / 2);
		vertices[vIndex].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertices[vIndex].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
		vertices[vIndex++].uv= XMFLOAT2(1.0, 0.0f);

		vertices[vIndex].position = XMFLOAT3(width / 2, 0.0f, -depth / 2);
		vertices[vIndex].normal= XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertices[vIndex].tangent = XMFLOAT3(1.0f, 0.0f, 0.0f);
		vertices[vIndex++].uv = XMFLOAT2(1.0, 1.0);
		std::vector<UINT> indices{0, 1, 2, 2, 3, 0};
		CreateBuffers(vertices, indices);
	}

	void component::Mesh::Create2DQuad(float size)
	{
		using namespace DirectX;
		constexpr int n_vertices = 4;
		constexpr int stride = 4;  // 2 + 2

		static const float data[] = {
			// position        uv
			-1.0f, -1.0f,  0.0f, 1.0f,
			+1.0f, -1.0f,  1.0f, 1.0f,
			+1.0f, +1.0f,  1.0f, 0.0f,
			-1.0f, +1.0f,  0.0f, 0.0f
		};

		std::vector<Vertex> vertices;
		vertices.reserve(n_vertices);

		for (unsigned int i = 0; i < n_vertices; i++) {
			unsigned int offset = i * stride;
			Vertex vertex{};
			vertex.position = XMFLOAT3(data[offset + 0], data[offset + 1], 0.0f) * size;
			vertex.uv = XMFLOAT2(data[offset + 2], data[offset + 3]);
			vertices.push_back(vertex);
		}

		// counter-clockwise winding order
		std::vector<UINT> indices{ 2, 1,0 , 0, 3, 2 };

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreateTorus(float R, float r)
	{
		using namespace DirectX;
		// default LOD = 60x60 faces, step size = 6 degrees
		UINT n_rings = 60;
		UINT n_sides = 60;
		UINT n_faces = n_sides * n_rings;  // quad face (2 triangles)
		UINT n_verts = n_sides * n_rings + n_sides;

		float delta_phi = util::PI*2 / n_rings;
		float delta_theta = util::PI * 2 / n_sides;

		std::vector<Vertex> vertices;
		vertices.reserve(n_verts);

		for (UINT ring = 0; ring <= n_rings; ring++) {
			float phi = ring * delta_phi;
			float cos_phi = cos(phi);
			float sin_phi = sin(phi);

			for (UINT side = 0; side < n_sides; side++) {
				float theta = side * delta_theta;
				float cos_theta = cos(theta);
				float sin_theta = sin(theta);

				float d = (R + r * cos_theta);  // distance from the vertex to the torus center

				float x = d * cos_phi;
				float y = d * sin_phi;
				float z = r * sin_theta;

				float a = d * cos_theta * cos_phi;
				float b = d * cos_theta * sin_phi;
				float c = d * sin_theta;

				float u =util::PI_2 * phi;
				float v = util::PI_2 * theta;

				Vertex vertex{};
				vertex.position = XMFLOAT3(x, y, z);
				vertex.normal = util::NormalizeXMFLOAT3(XMFLOAT3(a, b, c));
				vertex.uv = XMFLOAT2(u, v);

				vertices.push_back(vertex);
			}
		}

		std::vector<UINT> indices;
		indices.reserve(n_faces * 6);

		for (UINT ring = 0; ring < n_rings; ring++) {
			UINT offset = n_sides * ring;

			for (UINT side = 0; side < n_sides; side++) {
				UINT next_side = (side + 1) % n_sides;

				indices.push_back(offset + n_sides + next_side);
				indices.push_back(offset + n_sides + side);
				indices.push_back(offset + side);

				indices.push_back(offset + next_side);
				indices.push_back(offset + next_side + n_sides);
				indices.push_back(offset + side);
			}
		}

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreateCapsule(float a, float r)
	{
		using namespace DirectX;
		constexpr float n_rows = 100.0f;
		constexpr float n_cols = 100.0f;
		constexpr float PI = 3.141592654;
		constexpr float PI_2 = 3.141592654 / 2;

		float half_a = a / 2;  // half the cylinder height

		auto cylinder = [&](float u, float v) -> Vertex {
			float x = cos(PI * 2.0f * u) * r;
			float z = sin(PI * 2.0f * u) * r * (-1);
			float y = (v - 0.5f) * a;  // ~ [-a/2, a/2]

			Vertex vertex{};
			vertex.position = XMFLOAT3(x, y, z);
			vertex.normal = util::NormalizeXMFLOAT3(XMFLOAT3(x, 0.0f, z));
			vertex.uv = XMFLOAT2(u, v);
			vertex.binormal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertex.tangent = util::Cross(vertex.binormal, vertex.normal);
			return vertex;
		};

		auto lower_hemisphere = [&](float u, float v) -> Vertex {
			float phi = (PI * 2.0f) * u;        // ~ [0, 2PI]
			float theta = (PI / 2.0f) * (v - 1);  // ~ [-PI/2, 0]
			float x = cos(phi) * cos(theta) * r;
			float z = sin(phi) * cos(theta) * r * (-1);
			float y = sin(theta) * r;

			theta += PI_2;
			float r = cos(phi) * cos(theta);
			float t = sin(phi) * cos(theta) * (-1);
			float s = sin(theta);

			Vertex vertex{};
			vertex.position = XMFLOAT3(x, y - half_a, z);
			vertex.normal = util::NormalizeXMFLOAT3(XMFLOAT3(x, y, z));
			vertex.uv = XMFLOAT2(u, v);
			vertex.binormal = XMFLOAT3(r, s, t);
			vertex.tangent = util::Cross(vertex.binormal, vertex.normal);

			return vertex;
		};

		auto upper_hemisphere = [&](float u, float v) -> Vertex {
			float phi = (PI * 2.0f) * u;  // ~ [0, 2PI]
			float theta = (PI / 2.0f) * v;  // ~ [0, PI/2]
			float x = cos(phi) * cos(theta) * r;
			float z = sin(phi) * cos(theta) * r * (-1);
			float y = sin(theta) * r;

			theta += PI_2;
			float r = cos(phi) * cos(theta);
			float t = sin(phi) * cos(theta) * (-1);
			float s = sin(theta);

			Vertex vertex{};
			vertex.position = XMFLOAT3(x, y + half_a, z);
			vertex.normal = util::NormalizeXMFLOAT3(XMFLOAT3(x, y, z));
			vertex.uv = XMFLOAT2(u, v);
			vertex.binormal = XMFLOAT3(r, s, t);
			vertex.tangent = util::Cross(vertex.binormal, vertex.normal);

			return vertex;
		};

		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		vertices.reserve(static_cast<int>(n_rows * n_cols) * 12);
		indices.reserve(static_cast<int>(n_rows * n_cols) * 18);

		auto insert_indices = [&indices](auto&&... e) {
			(indices.push_back(std::forward<decltype(e)>(e)), ...);
		};

		unsigned int index = 0;

		for (float col = 0; col < n_cols; ++col) {
			for (float row = 0; row < n_rows; ++row) {
				float u0 = col / n_cols;
				float v0 = row / n_rows;
				float u1 = u0 + (1 / n_cols);
				float v1 = v0 + (1 / n_rows);

				// construct cylinder
				vertices.push_back(cylinder(u0, v0));  // push back rvalue, implicitly moved
				vertices.push_back(cylinder(u0, v1));
				vertices.push_back(cylinder(u1, v0));
				vertices.push_back(cylinder(u1, v1));

				insert_indices(index , index + 1, index + 2);  // 1st triangle
				insert_indices(index + 3, index + 2, index + 1);  // 2nd triangle
				index += 4;

				// construct lower hemisphere
				vertices.push_back(lower_hemisphere(u0, v0));
				vertices.push_back(lower_hemisphere(u0, v1));
				vertices.push_back(lower_hemisphere(u1, v0));
				vertices.push_back(lower_hemisphere(u1, v1));

				insert_indices(index  , index + 1, index + 2);  // 1st triangle
				insert_indices(index + 3, index + 2, index + 1);  // 2nd triangle
				index += 4;

				// construct upper hemisphere
				vertices.push_back(upper_hemisphere(u0, v0));
				vertices.push_back(upper_hemisphere(u0, v1));
				vertices.push_back(upper_hemisphere(u1, v0));
				vertices.push_back(upper_hemisphere(u1, v1));

				insert_indices(index , index + 1, index + 2);  // 1st triangle
				insert_indices(index + 3, index + 2, index + 1);  // 2nd triangle
				index += 4;
			}
		}

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreatePyramid(float s)
	{
		using namespace DirectX;
		constexpr int n_vertices = 16;  // really just 5 vertices but 16 normal directions
		constexpr int stride = 8;  // 3 + 3 + 2

		std::vector<Vertex> vertices;
		vertices.reserve(n_vertices);

		// mesh data precomputed in Blender
		static const float data[] = {
			// ----position----   --------------normal--------------   ---------uv---------
			+0.5f, +0.0f, +0.5f,  +0.000000f, -1.000000f, -0.000000f,  0.224609f, 0.390625f,
			-0.5f, +0.0f, +0.5f,  +0.000000f, -1.000000f, -0.000000f,  0.656250f, 0.390625f,
			+0.5f, +0.0f, -0.5f,  +0.000000f, -1.000000f, -0.000000f,  0.224609f, 0.816406f,
			-0.5f, +0.0f, -0.5f,  +0.000000f, -1.000000f, -0.000000f,  0.656250f, 0.816406f,
			+0.5f, +0.0f, +0.5f,  +0.894406f, +0.447188f, -0.000000f,  0.222656f, 0.390625f,
			+0.5f, +0.0f, -0.5f,  +0.894406f, +0.447188f, -0.000000f,  0.000000f, 0.000000f,
			+0.0f, +1.0f, +0.0f,  +0.894406f, +0.447188f, -0.000000f,  0.445313f, 0.000000f,
			-0.5f, +0.0f, +0.5f,  +0.000000f, +0.447188f, +0.894406f,  0.653863f, 0.377007f,
			+0.5f, +0.0f, +0.5f,  +0.000000f, +0.447188f, +0.894406f,  0.223340f, 0.379275f,
			+0.0f, +1.0f, +0.0f,  +0.000000f, +0.447188f, +0.894406f,  0.442318f, 0.000000f,
			+0.5f, +0.0f, -0.5f,  +0.000000f, +0.447188f, -0.894406f,  0.447266f, 0.000000f,
			-0.5f, +0.0f, -0.5f,  +0.000000f, +0.447188f, -0.894406f,  0.882812f, 0.000000f,
			+0.0f, +1.0f, +0.0f,  +0.000000f, +0.447188f, -0.894406f,  0.656250f, 0.376953f,
			-0.5f, +0.0f, -0.5f,  -0.894406f, +0.447188f, -0.000000f,  0.000000f, 0.000000f,
			-0.5f, +0.0f, +0.5f,  -0.894406f, +0.447188f, -0.000000f,  0.226638f, 0.386567f,
			+0.0f, +1.0f, +0.0f,  -0.894406f, +0.447188f, -0.000000f,  0.446560f, 0.000000f,
		};

		for (unsigned int i = 0; i < n_vertices; i++) {
			unsigned int offset = i * stride;

			Vertex vertex{};
			vertex.position = XMFLOAT3(data[offset + 0], data[offset + 1], data[offset + 2]) * s;
			vertex.normal = XMFLOAT3(data[offset + 3], data[offset + 4], data[offset + 5]);
			vertex.uv = XMFLOAT2(data[offset + 6], data[offset + 7]);
			vertex.normal = util::NormalizeXMFLOAT3(vertex.normal);

			vertices.push_back(vertex);
		}

		// counter-clockwise winding order
		std::vector<UINT> indices{
			+1, +0, +2, +3, +1, +2, +6, +5, +4,
			+9, +8, +7, 12, 11, 10, 15, 14, 13
		};

		CreateBuffers(vertices, indices);
	}

	void component::Mesh::CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
	{
		n_verts=vertices.size(), n_tris=indices.size();
		CD3D11_BUFFER_DESC bufferDesc(0,D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_DEFAULT,0);
		D3D11_SUBRESOURCE_DATA initData{ nullptr, 0, 0 };
		initData.pSysMem = vertices.data();
		bufferDesc.ByteWidth = (uint32_t)vertices.size() * sizeof(Vertex);
		global::GetGraphicI().m_pDevice->CreateBuffer(&bufferDesc, &initData, m_pVertices.GetAddressOf());
		initData.pSysMem = indices.data();
		bufferDesc = CD3D11_BUFFER_DESC((uint32_t)indices.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
		global::GetGraphicI().m_pDevice->CreateBuffer(&bufferDesc, &initData, m_pIndices.GetAddressOf());
	}

	component::Mesh::Mesh(Primitive object)
	{
		switch (object)
		{
		case component::Primitive::Sphere: {
			CreateSphere();
		}
			break;
		case component::Primitive::Cube:
			CreateCube();
			break;
		case component::Primitive::Plane:
			CreatePlane();
			break;
		case component::Primitive::Quad2D:
			Create2DQuad();
			break;
		case component::Primitive::Torus:
			CreateTorus();
			break;
		case component::Primitive::Capsule:
			CreateCapsule();
			break;
		case component::Primitive::Tetrahedron:
			CreatePyramid();
			break;
		default:
			break;
		}
	}

	component::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT32>& indices)
	{
		CreateBuffers(vertices, indices);
	}

	component::Mesh::Mesh(const std::shared_ptr<Mesh>& mesh_asset)
	{
	}

	void component::Mesh::Draw() const
	{

		auto E = global::GetGraphicI().m_pDeviceContext;
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		E->IASetVertexBuffers(0, 1, m_pVertices.GetAddressOf(), &stride, &offset);
		E->IASetIndexBuffer(m_pIndices.Get(), DXGI_FORMAT_R32_UINT , 0);
		E->DrawIndexed(n_tris, 0, 0);
	}
	void Mesh::DrawQuad()
	{
		static component::Mesh mesh(component::Primitive::Quad2D);
		mesh.Draw();
	}
	void Mesh::SetMaterialID(UINT mid) const
	{
		material_id = mid;
	}
}
