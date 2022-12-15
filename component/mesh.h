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

    class Mesh  {
    public:
        struct Vertex {
            DirectX::XMFLOAT3  position;
            DirectX::XMFLOAT3  normal;
            DirectX::XMFLOAT2  uv;
            DirectX::XMFLOAT2  uv2;
            DirectX::XMFLOAT3  tangent;
            DirectX::XMFLOAT3  binormal;
            DirectX::XMINT4 bone_id;
            DirectX::XMFLOAT4  bone_wt;  // the weight of each bone
        };

        static_assert(sizeof(Vertex) == 20 * sizeof(float) + 4 * sizeof(int));
        //顶点个数和索引数
        size_t n_verts, n_tris;

    private:
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
        Mesh(Primitive object);
        Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT32>& indices);
        Mesh(const std::shared_ptr<Mesh>& mesh_asset);

        void Draw() const;
        //static void DrawQuad();
        //static void DrawGrid();

        // this field is only used by meshes that are loaded from external models
        mutable UINT material_id;
        //void SetMaterialID(UINT mid) const;
    };
}
