#pragma once
#include"../scene/scene.h"
#include"../Texture2D.h"
namespace scene{
	class Scene01 :public Scene {
    public:
        Scene01(std::string name) :Scene(name){}
        void Init() override;
        void OnSceneRender() override;
        void OnImGuiRender() override;
        void PrecomputeIBL(const std::string& hdri);
        void SetupMaterial(component::Material& pbr_mat, int mat_id);
        TextureCube* skybox = nullptr;
        TextureCube* irradiance = nullptr;
        TextureCube* prefilter_map = nullptr;
        Texture2D* BRDF_LUT = nullptr;
        Entity camera;
        Entity sky;
        Entity sphere;

    };
}