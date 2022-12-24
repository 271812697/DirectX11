#pragma once
#include"../scene/scene.h"
#include"../asset/Texture2D.h"
namespace scene{
    using namespace asset;
	class Scene01 :public Scene {
    public:
        Scene01(std::string name) :Scene(name){}
        void Init() override;
        void OnSceneRender() override;
        void OnImGuiRender() override;
        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(component::Material& pbr_mat, int mat_id);
        std::shared_ptr<TextureCube> skybox = nullptr;
        std::shared_ptr < TextureCube> irradiance = nullptr;
        std::shared_ptr < TextureCube> prefilter_map = nullptr;
        std::shared_ptr < Texture2D>BRDF_LUT = nullptr;
        Entity camera;
        Entity sky;
        Entity sphere;

    };
}