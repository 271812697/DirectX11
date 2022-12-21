#include "mesh.h"
#include"../util/math.h"
#include"../util/global.h"
namespace component {
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
	}

	void component::Mesh::CreatePlane(float size)
	{
	}

	void component::Mesh::Create2DQuad(float size)
	{
	}

	void component::Mesh::CreateTorus(float R, float r)
	{
	}

	void component::Mesh::CreateCapsule(float a, float r)
	{
	}

	void component::Mesh::CreatePyramid(float s)
	{
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
			break;
		case component::Primitive::Plane:
			break;
		case component::Primitive::Quad2D:
			break;
		case component::Primitive::Torus:
			break;
		case component::Primitive::Capsule:
			break;
		case component::Primitive::Tetrahedron:
			break;
		default:
			break;
		}
	}

	component::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT32>& indices)
	{
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
}
