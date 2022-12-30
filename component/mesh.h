#pragma once
#include<stdint.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include<d3d11.h>
#include<vector>
#include <memory>
namespace component {
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    enum class Primitive : uint8_t {
        Sphere, Cube, Plane, Quad2D, Torus, Capsule, Tetrahedron
    };
    template<size_t numElements>
    using D3D11_INPUT_ELEMENT_DESC_ARRAY = const D3D11_INPUT_ELEMENT_DESC(&)[numElements];
    class Mesh  {
    public:
        //static ComPtr<ID3D11InputLayout> m_pVertexLayout;
        struct Vertex {
            DirectX::XMFLOAT3  position;
            DirectX::XMFLOAT3  normal;
            DirectX::XMFLOAT2  uv;
            DirectX::XMFLOAT2  uv2;
            DirectX::XMFLOAT3  tangent;
            DirectX::XMFLOAT3  binormal;
            DirectX::XMINT4 bone_id;
            DirectX::XMFLOAT4  bone_wt;  // the weight of each bone
            static D3D11_INPUT_ELEMENT_DESC_ARRAY<8> GetInputLayout()
            {
                static const D3D11_INPUT_ELEMENT_DESC inputLayout[8] = {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD", 1,DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TANGENT", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 } ,
                    { "BINORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "BONEID", 0,DXGI_FORMAT_R32G32B32A32_SINT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "BONEWE", 0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };
                return inputLayout;
            }
            	    // 顶点输入布局
        };
        static_assert(sizeof(Vertex) == 20 * sizeof(float) + 4 * sizeof(int));
        //顶点个数和索引数
        size_t n_verts, n_tris;

    public:
        //friend class Model;
        ComPtr<ID3D11Buffer> m_pVertices;
        ComPtr<ID3D11Buffer> m_pIndices;


        void CreateSphere(float radius = 1.0f);
        void CreateCube(float size = 1.0f);
        void CreatePlane(float size = 10.0f);
        void Create2DQuad(float size = 1.0f);
        void CreateTorus(float R = 1.5f, float r = 0.5f);
        void CreateCapsule(float a = 2.0f, float r = 1.0f);
        void CreatePyramid(float s = 2.0f);
        void CreateBuffers(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices);

    public:
        Mesh() = default;
        Mesh(Primitive object);
        Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT32>& indices);
        Mesh(const std::shared_ptr<Mesh>& mesh_asset);

        void Draw() const;
        static void DrawQuad();
        //static void DrawGrid();

        // this field is only used by meshes that are loaded from external models
        mutable UINT material_id;
        //void SetMaterialID(UINT mid) const;
    };
}
